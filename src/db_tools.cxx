#include "xray_re/xr_file_system.hxx"
#include "db_tools.hxx"
#include "unpacker.hxx"
#include "packer.hxx"

#include <boost/range/adaptors.hpp>
#include <boost/range/adaptor/type_erased.hpp>
#include <boost/range/iterator_range.hpp>
#include <spdlog/spdlog.h>

#include <filesystem>

using namespace xray_re;

bool m_debug = false;

void DBTools::unpack(const std::string& source_path, const std::string& destination_path, const xray_re::DBVersion& version, const std::string& filter)
{
	Unpacker unpacker;
	unpacker.process(source_path, destination_path, version, filter);
}


void DBTools::pack(
	const std::string& source_path,
	std::string& destination_path,
	const xray_re::DBVersion& version,
	const std::string& xdb_ud,
	const bool& dont_strip,
	const bool& skip_folders,
	boost::regex& expression,
	const size_t& max_size,
	const bool& save_list
)
{
	auto options = new PackerOptions;

	options->m_source_path = source_path;
	options->m_destination_path = destination_path;
	options->m_version = version;
	options->m_xdb_ud = xdb_ud;
	options->m_dont_strip = dont_strip;
	options->m_skip_folders = skip_folders;
	options->m_expression = expression;
	options->m_max_size = max_size;
	options->m_save_list = save_list;

	Packer packer(options);
	packer.process();
}

void DBTools::packSplitted(
	const std::string& source_path,
	std::string& destination_path,
	const xray_re::DBVersion& version,
	const std::string& xdb_ud,
	const bool& dont_strip,
	const bool& skip_folders,
	boost::regex& expression,
	const size_t& max_size,
	const bool& save_list
)
{
	if(!xr_file_system::folder_exist(source_path))
	{
		spdlog::error("Failed to find folder {}", source_path);
		return;
	}

	int i = 1;
	size_t current_max_size = 0;
	boost::smatch what;
	std::vector<std::string> files;

	for (auto& entry : std::filesystem::recursive_directory_iterator(source_path))
	{
		if(!entry.is_regular_file())
			continue;

		if(boost::regex_match(std::filesystem::path(entry).string(), what, expression))
			continue;

		if (current_max_size >= max_size)
		{
			char buf [256];
			sprintf(buf, "_%05d.db", i);
			auto temp_path = destination_path;
			temp_path.replace(temp_path.length() - 3, 4, buf);

			auto options = new PackerOptions;

			options->m_input_files = files;
			options->m_destination_path = destination_path;
			options->m_version = version;
			options->m_xdb_ud = xdb_ud;
			options->m_dont_strip = dont_strip;
			options->m_skip_folders = skip_folders;
			options->m_save_list = save_list;

			Packer packer(options);
			packer.processFiles();

			files.clear();
			current_max_size = 0;
			i += 1;
		}

		current_max_size += entry.file_size();
		files.push_back(entry.path().string());
	}

	if (current_max_size >= 0)
	{
		char buf [256];
		sprintf(buf, "_%05d.db", i);
		destination_path.replace(destination_path.length() - 3, 4, buf);

		auto options = new PackerOptions;

		options->m_input_files = files;
		options->m_destination_path = destination_path;
		options->m_version = version;
		options->m_xdb_ud = xdb_ud;
		options->m_dont_strip = dont_strip;
		options->m_skip_folders = skip_folders;
		options->m_save_list = save_list;

		Packer packer(options);
		packer.processFiles();

		files.clear();
		current_max_size = 0;
	}
}

void DBTools::packFiles(
	const std::vector<std::string>& files,
	const std::string& destination_path,
	const xray_re::DBVersion& version,
	const std::string& xdb_ud,
	const bool& dont_strip,
	const bool& skip_folders
)
{
	auto options = new PackerOptions;

	options->m_input_files = files;
	options->m_destination_path = destination_path;
	options->m_version = version;
	options->m_xdb_ud = xdb_ud;
	options->m_dont_strip = dont_strip;
	options->m_skip_folders = skip_folders;

	Packer packer(options);
	packer.processFiles();
}


bool DBTools::is_xrp(const std::string& extension)
{
	return extension == ".xrp";
}

bool DBTools::is_xp(const std::string& extension)
{
	return (extension.size() == 3 && extension == ".xp") ||
	       (extension.size() == 4 && extension.compare(0, 3, ".xp") == 0 && std::isalnum(extension[3]));
}

bool DBTools::is_xdb(const std::string& extension)
{
	return (extension.size() == 3 && extension == ".xdb") ||
	       (extension.size() == 4 && extension.compare(0, 3, ".xdb") == 0 && std::isalnum(extension[4]));
}

bool DBTools::is_db(const std::string& extension)
{
	return (extension.size() == 3 && extension == ".db") ||
	       (extension.size() == 4 && extension.compare(0, 3, ".db") == 0 && std::isalnum(extension[3]));
}

bool DBTools::is_known(const std::string& extension)
{
	return is_db(extension) || is_xdb(extension) || is_xrp(extension) || is_xp(extension);
}

void DBTools::set_debug(const bool value)
{
	m_debug = value;
	if(m_debug)
	{
		spdlog::set_level(spdlog::level::debug);
	}
}
