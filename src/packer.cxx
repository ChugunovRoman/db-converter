#include "packer.hxx"
#include "db_tools.hxx"

#include "xray_re/xr_file_system.hxx"
#include "xray_re/xr_utils.hxx"
#include "xray_re/xr_lzhuf.hxx"
#include "xray_re/xr_scrambler.hxx"
#include "crc32/crc32.hxx"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/adaptor/type_erased.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string/join.hpp>
#include <spdlog/spdlog.h>

#include <filesystem>

using namespace xray_re;

extern bool m_debug;

Packer::~Packer()
{
	m_files.clear();
	delete_elements(m_files);
}

template <typename... Args>
	auto Packer::make_directory_range(bool recursive, Args... args) {
		return recursive
			? boost::make_iterator_range(std::filesystem::recursive_directory_iterator(args...), std::filesystem::recursive_directory_iterator()) | boost::adaptors::type_erased()
			: boost::make_iterator_range(std::filesystem::directory_iterator(args...), std::filesystem::directory_iterator());
}


void Packer::process()
{
	if(m_options->m_source_path.empty())
	{
		spdlog::error("Missing source directory path");
		return;
	}

	if(!xr_file_system::folder_exist(m_options->m_source_path))
	{
		spdlog::error("Failed to find folder {}", m_options->m_source_path);
		return;
	}

	if(m_options->m_destination_path.empty())
	{
		spdlog::error("Missing destination file path");
		return;
	}

	xr_file_system& fs = xr_file_system::instance();
	auto path_splitted = fs.split_path(m_options->m_destination_path);

	if(!xr_file_system::folder_exist(path_splitted.folder))
	{
		spdlog::info("Destination folder {} doesn't exist, creating", path_splitted.folder);
		fs.create_path(path_splitted.folder);
	}

	if(m_options->m_version == DBVersion::DB_VERSION_AUTO)
	{
		spdlog::error("Unspecified DB format");
		return;
	}

	if(m_options->m_version == DBVersion::DB_VERSION_1114 || m_options->m_version == DBVersion::DB_VERSION_2215 || m_options->m_version == DBVersion::DB_VERSION_2945)
	{
		spdlog::error("Unsupported DB format");
		return;
	}

	init_archive();

	fs.append_path_separator(m_root);

	m_archive->open_chunk(DB_CHUNK_DATA);
	m_root = m_options->m_source_path;
	fs.append_path_separator(m_root);
	process_folder(m_root, m_options->m_dont_strip, m_options->m_skip_folders, m_options->m_expression, m_options->m_max_size);
	m_archive->close_chunk();

	pack();
}

void Packer::init_archive()
{
	xr_file_system& fs = xr_file_system::instance();
	m_archive = fs.w_open(m_options->m_destination_path);

	if(!m_archive)
	{
		spdlog::error("Failed to load {}", m_options->m_destination_path);
		return;
	}

	if(m_options->m_version == DBVersion::DB_VERSION_XDB && !m_options->m_xdb_ud.empty())
	{
		if(auto reader = fs.r_open(m_options->m_xdb_ud))
		{
			m_archive->open_chunk(DB_CHUNK_USERDATA);
			m_archive->w_raw(reader->data(), reader->size());
			m_archive->close_chunk();
			fs.r_close(reader);
		}
		else
		{
			spdlog::error("Failed to load {}", m_options->m_xdb_ud);
		}
	}
}

void Packer::processFiles()
{
	if(m_options->m_destination_path.empty())
	{
		spdlog::error("Missing destination file path");
		return;
	}

	xr_file_system& fs = xr_file_system::instance();
	auto path_splitted = fs.split_path(m_options->m_destination_path);

	if(!xr_file_system::folder_exist(path_splitted.folder))
	{
		spdlog::info("Destination folder {} doesn't exist, creating", path_splitted.folder);
		fs.create_path(path_splitted.folder);
	}

	if(m_options->m_version == DBVersion::DB_VERSION_AUTO)
	{
		spdlog::error("Unspecified DB format");
		return;
	}

	if(m_options->m_version == DBVersion::DB_VERSION_1114 || m_options->m_version == DBVersion::DB_VERSION_2215 || m_options->m_version == DBVersion::DB_VERSION_2945)
	{
		spdlog::error("Unsupported DB format");
		return;
	}

	fs.append_path_separator(m_root);

	m_archive = fs.w_open(m_options->m_destination_path);
	if(!m_archive)
	{
		spdlog::error("Failed to load {}", m_options->m_destination_path);
		return;
	}

	if(m_options->m_version == DBVersion::DB_VERSION_XDB && !m_options->m_xdb_ud.empty())
	{
		if(auto reader = fs.r_open(m_options->m_xdb_ud))
		{
			m_archive->open_chunk(DB_CHUNK_USERDATA);
			m_archive->w_raw(reader->data(), reader->size());
			m_archive->close_chunk();
			fs.r_close(reader);
		}
		else
		{
			spdlog::error("Failed to load {}", m_options->m_xdb_ud);
		}
	}

	m_archive->open_chunk(DB_CHUNK_DATA);

	for(const auto& file_path : m_options->m_input_files)
	{
		if(!xr_file_system::file_exist(file_path))
		{
			spdlog::error("Failed to find file {} Skipped", file_path);
			continue;
		}

		process_file(file_path, m_options->m_dont_strip);
	}

	m_archive->close_chunk();

	pack();
}

void Packer::pack()
{
	spdlog::info("files: ");
	xr_file_system& fs = xr_file_system::instance();
	auto w = new xr_memory_writer;
	std::vector<std::string> json_files;

	for(const auto& file : m_files)
	{
		std::replace(file->path.begin(), file->path.end(), '/', '\\');
		w->w_size_u16(file->path.size() + 16);
		w->w_size_u32(file->size_real);
		w->w_size_u32(file->size_compressed);
		w->w_u32(file->crc);
		w->w_raw(file->path.data(), file->path.size());
		spdlog::info("  {}", file->path);
		w->w_size_u32(file->offset);

		if (m_options->m_save_list)
			json_files.push_back(escape_json(file->path));
	}

	uint8_t *data = nullptr;
	uint32_t size = 0;
	xr_lzhuf::compress(data, size, w->data(), w->tell());
	delete w;

	if(m_options->m_version == DBVersion::DB_VERSION_2947RU)
	{
		xr_scrambler scrambler(xr_scrambler::CC_RU);
		scrambler.encrypt(data, data, size);
	}
	else if(m_options->m_version == DBVersion::DB_VERSION_2947WW)
	{
		xr_scrambler scrambler(xr_scrambler::CC_WW);
		scrambler.encrypt(data, data, size);
	}

	m_archive->open_chunk(DB_CHUNK_HEADER | CHUNK_COMPRESSED);
	m_archive->w_raw(data, size);
	m_archive->close_chunk();

	if (m_options->m_save_list)
	{
		std::string json_file_name = m_options->m_destination_path;

		json_file_name.insert(json_file_name.length(), ".json");

		boost::filesystem::ofstream json_file(json_file_name);
		json_file << "[\"";
		json_file << boost::algorithm::join(json_files, "\",\"");
		json_file << "\"]";
		json_file.close();
	}

	delete data;
	fs.w_close(m_archive);
}

void Packer::process_folder(const std::string& path, const bool& dont_strip, const bool& skip_folders, const boost::regex& expression, const size_t& max_size)
{
	boost::smatch what;
	std::vector<std::filesystem::directory_entry> files, folders;

	for(auto& entry : make_directory_range(!skip_folders, path))
	{
		if(entry.is_directory() && !skip_folders)
		{
			folders.emplace_back(entry);
			continue;
		}

		if(!entry.is_regular_file())
			continue;

		if(boost::regex_match(std::filesystem::path(entry).string(), what, expression))
			continue;

		files.emplace_back(entry);
	}

	auto comparator = [](auto lhs, auto rhs)
	{
		return lhs.path() < rhs.path();
	};

	std::sort(folders.begin(), folders.end(), comparator);

	auto root_path = std::filesystem::path(path);

	for(const auto& folder : folders)
	{
		auto entry_path = std::filesystem::path(folder);
		auto relative_path = std::filesystem::relative(entry_path, root_path);
		if (dont_strip)
		{
			relative_path = entry_path;
		}
		m_folders.push_back(relative_path);
	}

	std::sort(files.begin(), files.end(), comparator);

	for(const auto& file : files)
	{
		auto entry_path = std::filesystem::path(file);
		auto relative_path = std::filesystem::relative(entry_path, root_path);
		if (dont_strip)
		{
			relative_path = entry_path;
		}
		process_file(relative_path, dont_strip);
	}
}

void Packer::process_file(const std::string& path, const bool& dont_strip)
{
	constexpr bool compress_files = false;

	if constexpr(compress_files)
	{
		xr_file_system& fs = xr_file_system::instance();
		auto m_path = path;
		if (!dont_strip)
		{
			m_path = m_root + path;
		}
		auto reader = fs.r_open(m_path);
		if(reader)
		{
			auto data = static_cast<const uint8_t*>(reader->data());
			auto offset = m_archive->tell();
			auto size = reader->size();

			uint8_t *data_compressed = nullptr;
			uint32_t size_compressed = 0u;
			xr_lzhuf::compress(data_compressed, size_compressed, data, size);
			spdlog::info("{}->{} {}", size, size_compressed, path);

			auto crc = crc32(data_compressed, size_compressed);
			m_archive->w_raw(data_compressed, size_compressed);
			fs.r_close(reader);

			auto path_lowercase = path;
			std::transform(path_lowercase.begin(), path_lowercase.end(), path_lowercase.begin(), [](unsigned char c) { return std::tolower(c); });
			fs.trim_start_path_separator(path_lowercase);

			auto file = new db_file;
			file->path = path_lowercase;
			file->crc = crc;
			file->offset = offset;
			file->size_real = size;
			file->size_compressed = size_compressed;
			m_files.push_back(file);
		}
	}
	else
	{
		xr_file_system& fs = xr_file_system::instance();
		auto m_path = path;
		if (!dont_strip)
		{
			m_path = m_root + path;
		}
		auto reader = fs.r_open(m_path);
		if(reader)
		{
			auto offset = m_archive->tell();
			auto size = reader->size();
			auto crc = crc32(reader->data(), size);
			m_archive->w_raw(reader->data(), size);
			fs.r_close(reader);

			std::string path_lowercase = path;
			std::transform(path_lowercase.begin(), path_lowercase.end(), path_lowercase.begin(), [](unsigned char c) { return std::tolower(c); });
			fs.trim_start_path_separator(path_lowercase);

			auto file = new db_file;
			file->path = path_lowercase;
			file->crc = crc;
			file->offset = offset;
			file->size_real = size;
			file->size_compressed = size;
			m_files.push_back(file);
		}
	}
}

std::string Packer::escape_json(const std::string &s)
{
	std::ostringstream o;
	for (auto c = s.cbegin(); c != s.cend(); c++) {
		if (*c == '"') {
			o << "\\\"";
		} else if(*c == '\\') {
			o << "\\\\";
		} else {
			o << *c;
		}
	}

	return o.str();
}
