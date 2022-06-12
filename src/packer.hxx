#pragma once

#include "xray_re/xr_types.hxx"

#include <string>
#include <vector>

namespace xray_re
{
	class xr_reader;
	class xr_writer;
};

class Packer
{
public:
	~Packer();

	void process(const std::string& source_path, const std::string& destination_path, const xray_re::DBVersion& version, const std::string& xdb_ud, const bool& dont_strip, const bool& skip_folders);
	void processFiles(const std::vector<std::string>& files, const std::string& destination_path, const xray_re::DBVersion& version, const std::string& xdb_ud, const bool& dont_strip, const bool& skip_folders);

private:
	void process_folder(const std::string& path = "", const bool& dont_strip = false, const bool& skip_folders = false);
	void process_file(const std::string& path, const bool& dont_strip);
	void add_folder(const std::string& path);

	void pack(const xray_re::DBVersion& version);

	template <typename... Args> auto make_directory_range(bool recursive, Args... args);

	xray_re::xr_writer *m_archive;
	std::string m_root;
	std::vector<std::string> m_folders;
	std::vector<xray_re::db_file*> m_files;
};
