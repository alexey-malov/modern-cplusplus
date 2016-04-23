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

#define BOOST_TEST_NO_MAIN 
#define BOOST_TEST_ALTERNATIVE_INIT_API 
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

namespace
{
typedef vector<LayerPtr> Layers;

struct A_Layer_
{
	const LayerPtr layer = make_shared<Layer>();
	const LayerPtr sublayer = make_shared<Layer>();

	const LayerPtr sublayerA = make_shared<Layer>();
	const LayerPtr sublayerB = make_shared<Layer>();
	const LayerPtr sublayerC = make_shared<Layer>();
	const LayerPtr sublayerD = make_shared<Layer>();
};

Layers GetSublayers(const Layer& layer)
{
	Layers sublayers;
	const size_t n = layer.GetSublayerCount();
	for (size_t i = 0; i < n; ++i)
	{
		sublayers.push_back(layer.GetSublayer(i));
	}
	return sublayers;
}

}

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
		A_Layer_with_one_sublayer_()
		{
			layer->AddSublayer(sublayer);
		}
	};
	BOOST_FIXTURE_TEST_SUITE(when_adding_a_sublayer, A_Layer_with_one_sublayer_)
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
		BOOST_AUTO_TEST_CASE(forbids_getting_a_sublayer_by_wrong_index)
		{
			BOOST_CHECK_THROW((void)&layer->GetSublayer(1);, out_of_range);
		}
		BOOST_AUTO_TEST_CASE(shifts_subsequent_sublayers)
		{
			auto frontSublayer = make_shared<Layer>();

			layer->InsertSublayerAtIndex(frontSublayer, 0);

			BOOST_REQUIRE(GetSublayers(*layer) == Layers({ frontSublayer, sublayer }));

			auto middleSublayer = make_shared<Layer>();
			layer->InsertSublayerAtIndex(middleSublayer, 1);
			BOOST_REQUIRE(GetSublayers(*layer) == Layers({ frontSublayer, middleSublayer, sublayer }));
		}
	BOOST_AUTO_TEST_SUITE_END()

	struct A_Layer_with_several_sublayers : A_Layer_
	{
		A_Layer_with_several_sublayers()
		{
			layer->AddSublayer(sublayerA);
			layer->AddSublayer(sublayerB);
			layer->AddSublayer(sublayerC);
			layer->AddSublayer(sublayerD);
		}
	};
	BOOST_FIXTURE_TEST_SUITE(when_adding_own_sublayer, A_Layer_with_several_sublayers)
		BOOST_AUTO_TEST_CASE(moves_it_to_the_destination_position)
		{
			// ABCD => DABC
			BOOST_REQUIRE_NO_THROW(layer->InsertSublayerAtIndex(sublayerD, 0));
			BOOST_REQUIRE(GetSublayers(*layer) == 
				Layers({sublayerD, sublayerA, sublayerB, sublayerC}));

			// DABC => ABCD
			BOOST_REQUIRE_NO_THROW(layer->InsertSublayerAtIndex(sublayerD, 4));
			BOOST_REQUIRE(GetSublayers(*layer) ==
				Layers({ sublayerA, sublayerB, sublayerC, sublayerD }));

			// ABCD => BCAD
			BOOST_REQUIRE_NO_THROW(layer->InsertSublayerAtIndex(sublayerA, 3));
			BOOST_REQUIRE(GetSublayers(*layer) ==
				Layers({ sublayerB, sublayerC, sublayerA, sublayerD }));

			// BC[A]D => BC[A]D
			BOOST_REQUIRE_NO_THROW(layer->InsertSublayerAtIndex(sublayerA, 2));
			BOOST_REQUIRE(GetSublayers(*layer) ==
				Layers({ sublayerB, sublayerC, sublayerA, sublayerD }));
		}
	BOOST_AUTO_TEST_SUITE_END()
	BOOST_AUTO_TEST_SUITE(does_not_allow_inserting)
		BOOST_AUTO_TEST_CASE(itself_as_its_own_sublayer)
		{
			auto sublayers = GetSublayers(*layer);
			BOOST_REQUIRE_THROW(layer->AddSublayer(layer), invalid_argument);
			BOOST_CHECK(!layer->GetSuperlayer());
			BOOST_CHECK(GetSublayers(*layer) == sublayers);
		}
		BOOST_AUTO_TEST_CASE(null_layer)
		{
			auto sublayers = GetSublayers(*layer);
			BOOST_REQUIRE_THROW(layer->AddSublayer(nullptr), invalid_argument);
			BOOST_CHECK(GetSublayers(*layer) == sublayers);
		}
	BOOST_AUTO_TEST_SUITE_END()

	struct A_layer_being_a_sublayer_ : A_Layer_
	{
		const LayerPtr superLayer = make_shared<Layer>();
		const LayerPtr topmostLayer = make_shared<Layer>();
		A_layer_being_a_sublayer_()
		{
			superLayer->AddSublayer(layer);
			topmostLayer->AddSublayer(superLayer);
		}
	};
	BOOST_FIXTURE_TEST_SUITE(being_a_sublayer, A_layer_being_a_sublayer_)
		BOOST_AUTO_TEST_CASE(can_be_removed_from_parent)
		{
			layer->RemoveFromSuperlayer();
			BOOST_CHECK(!layer->GetSuperlayer());
			BOOST_CHECK_EQUAL(superLayer->GetSublayerCount(), 0u);
		}
		BOOST_AUTO_TEST_CASE(does_not_allow_inserting_any_of_its_superlayers)
		{
			BOOST_CHECK_THROW(layer->AddSublayer(superLayer), std::invalid_argument);
			BOOST_CHECK_EQUAL(layer->GetSuperlayer(), superLayer);
			BOOST_CHECK_EQUAL(superLayer->GetSuperlayer(), topmostLayer);

			BOOST_CHECK_THROW(layer->AddSublayer(topmostLayer), std::invalid_argument);
			BOOST_CHECK_EQUAL(layer->GetSuperlayer(), superLayer);
			BOOST_CHECK(!topmostLayer->GetSuperlayer());
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
