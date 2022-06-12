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

	void process(const std::string& source_path, const std::string& destination_path, const xray_re::DBVersion& version, const std::string& xdb_ud, const bool& dont_strip);

private:
	void process_folder(const std::string& path = "", const bool& dont_strip = false);
	void process_file(const std::string& path, const bool& dont_strip);
	void add_folder(const std::string& path);

	xray_re::xr_writer *m_archive;
	std::string m_root;
	std::vector<std::string> m_folders;
	std::vector<xray_re::db_file*> m_files;
};
