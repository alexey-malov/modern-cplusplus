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

int main()
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

	//{
	//	using RecDirIt = recursive_directory_iterator;
	//	auto IsFile = [](const auto& dirEntry) {
	//		return is_regular_file(dirEntry);
	//	};

	//	auto dirEntries = make_iterator_range(RecDirIt(current_path()), RecDirIt());
	//	auto fileInfoRng = 
	//		dirEntries
	//		| filtered(IsFile)
	//		| transformed(bind(MakeFileInfo, placeholders::_1, current_path()));
	//	vector<FileInfo> fileInfos(fileInfoRng.begin(), fileInfoRng.end());
	//}
	return 0;
}
