#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <map>
#include <string>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/transform.hpp>
#include <functional>
#include <iterator>
#include <vector>
#include "Layer.h"

#include <boost/algorithm/string/replace.hpp>

//#define BOOST_TEST_MODULE MyTestModule
#define BOOST_TEST_NO_MAIN 
#define BOOST_TEST_ALTERNATIVE_INIT_API 
//#define BOOST_TEST_MODULE MyTestModule
#pragma warning (disable: 4702)
#pragma warning (push, 3)
#include <boost/test/unit_test.hpp>
#pragma warning (pop)

#pragma warning (push, 3)
#include <boost/test/included/unit_test.hpp>
#pragma warning (pop)

#include <boost/test/output/compiler_log_formatter.hpp>

using namespace std;
using namespace boost::filesystem;
using namespace boost;
using namespace boost::adaptors;

enum class FileType
{
	Image,
	Audio,
	Video,
	Text,
	Code,
	Other
};

struct FileInfo
{
	path path;
	FileType type;
	uintmax_t size;
};

template < >
boost::filesystem::path& boost::filesystem::path::append< typename boost::filesystem::path::iterator >(typename boost::filesystem::path::iterator begin, typename boost::filesystem::path::iterator end/*, const codecvt_type& cvt*/)
{
	for (; begin != end; ++begin)
		*this /= *begin;
	return *this;
}

path make_relative(path a_From, path a_To)
{
	a_From = absolute(a_From);
	a_To = absolute(a_To);
	path ret;
	auto itrFrom(a_From.begin());
	auto itrTo(a_To.begin());
	// Find common base
	for (path::const_iterator toEnd(a_To.end()), fromEnd(a_From.end()); itrFrom != fromEnd && itrTo != toEnd && *itrFrom == *itrTo; ++itrFrom, ++itrTo);
	// Navigate backwards in directory to reach previously found base
	for (path::const_iterator fromEnd(a_From.end()); itrFrom != fromEnd; ++itrFrom) //-V683
	{
		if ((*itrFrom) != ".")
			ret /= "..";
	}
	// Now navigate down the directory branch
	ret.append(itrTo, a_To.end());
	return ret;
}


FileInfo MakeFileInfo(const directory_entry& d, const path& basePath = current_path())
{
	static map<wstring, FileType> extToType = {
		{ L".png", FileType::Image },
		{ L".jpg", FileType::Image },
		{ L".mp3", FileType::Audio },
		{ L".ogg", FileType::Audio },
		{ L".mp4", FileType::Video },
		{ L".avi", FileType::Video },
		{ L".txt", FileType::Text },
		{ L".cpp", FileType::Code },
		{ L".h", FileType::Code },
	};
	auto const& p = d.path();
	auto it = extToType.find(p.extension().native());
	auto type = (it != extToType.end()) ? it->second : FileType::Other;
	return { make_relative(p, basePath), type, boost::filesystem::file_size(p)};
}

wstring ToWstring(FileType ft)
{
	static map<FileType, wstring> typeToWstring = {
		{ FileType::Image, L"image" },
		{ FileType::Audio, L"audio" },
		{ FileType::Video, L"video" },
		{ FileType::Text,  L"text" },
		{ FileType::Code,  L"code" },
		{ FileType::Other, L"other" },
	};
	return typeToWstring.at(ft);
}

wstring FileInfoToWstring(const FileInfo & info)
{
	return info.path.native() + L";" + ToWstring(info.type)
		+ L";" + to_wstring(info.size);
}

/*
Данный класс управляет формаитрованием журнала запуска тестов
Для того, чтобы увидеть результат, приложение должно быть запущено с ключём --log_level=test_suite (см. Post-build event в настройках проекта)
*/
class SpecLogFormatter :
	public boost::unit_test::output::compiler_log_formatter
{
	virtual void test_unit_start(std::ostream &os, boost::unit_test::test_unit const& tu) override
	{
		// перед запуском test unit-а выводим имя test unit-а, заменяя символ подчеркивания на пробел
		os << std::string(m_indent, ' ') << boost::replace_all_copy(tu.p_name.get(), "_", " ") << std::endl;
		// увеличиваем отступ для вывода имен последующих test unit-ов в виде дерева
		m_indent += 2;
	}

	virtual void test_unit_finish(std::ostream &/*os*/, boost::unit_test::test_unit const& /*tu*/, unsigned long /*elapsed*/) override
	{
		// по окончании test unit-а уменьшаем отступ
		m_indent -= 2;
	}

	size_t m_indent = 0;
};

bool init_unit_test()
{
	// Заменили имя log formatter на пользовательский
	boost::unit_test::unit_test_log.set_formatter(new SpecLogFormatter);
	// Имя корневого набора тестов - All tests
	boost::unit_test::framework::master_test_suite().p_name.value = "All tests";
	return true;
}

struct A_Layer_
{
	const LayerPtr layer = make_shared<Layer>();
};

BOOST_FIXTURE_TEST_SUITE(A_Layer, A_Layer_)
	BOOST_AUTO_TEST_SUITE(when_created)
		BOOST_AUTO_TEST_CASE(has_no_parent)
		{
			BOOST_CHECK(!layer->GetSuperlayer());
		}
		BOOST_AUTO_TEST_CASE(has_no_children)
		{
			BOOST_CHECK_EQUAL(layer->GetSublayerCount(), 0u);
		}
	BOOST_AUTO_TEST_SUITE_END()
	
	struct A_Layer_with_one_sublayer_ : A_Layer_
	{
		const LayerPtr sublayer = make_shared<Layer>();
		A_Layer_with_one_sublayer_()
		{
			layer->AddSublayer(sublayer);
		}
	};
	BOOST_FIXTURE_TEST_SUITE(after_adding_a_sublayer, A_Layer_with_one_sublayer_)
		BOOST_AUTO_TEST_CASE(contains_one_sublayer)
		{
			BOOST_CHECK_EQUAL(layer->GetSublayerCount(), 1u);
		}
		BOOST_AUTO_TEST_CASE(becomes_a_superlayer_of_the_added_sublayer)
		{
			BOOST_CHECK_EQUAL(layer, sublayer->GetSuperlayer());
		}
		BOOST_AUTO_TEST_CASE(contains_the_added_sublayer_at_index_0)
		{
			BOOST_CHECK_EQUAL(layer->GetSublayer(0), sublayer);
		}
	BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()



int main(int argc, char* argv[])
{

	{
		vector<FileInfo> fileInfos;
		for (recursive_directory_iterator it(current_path()); it != recursive_directory_iterator(); ++it)
		{
			if (is_regular_file(it->status()))
			{
				fileInfos.push_back(MakeFileInfo(*it));
			}
		}
	}

	{
		using RecDirIt = recursive_directory_iterator;
		auto IsFile = [](const auto& dirEntry) {
			return is_regular_file(dirEntry);
		};

		auto dirEntries = make_iterator_range(RecDirIt(current_path()), RecDirIt());
		auto fileInfoRng = dirEntries 
			| filtered(IsFile) 
			| transformed(bind(MakeFileInfo, boost::placeholders::_1, current_path()));
		vector<FileInfo> fileInfos(fileInfoRng.begin(), fileInfoRng.end());
	}
	// Запускаем тесты, передавая параметры командной строки и кастомную функцию инициализации тестов
	return boost::unit_test::unit_test_main(init_unit_test, argc, argv);
}
