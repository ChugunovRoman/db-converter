#pragma once

#include "xray_re/xr_types.hxx"

#include <boost/regex.hpp>
#include <string>
#include <vector>

namespace xray_re
{
	class xr_reader;
	class xr_writer;
};

struct PackerOptions
{
	std::string m_source_path;
	std::string m_destination_path;
	std::vector<std::string> m_input_files;
	xray_re::DBVersion m_version;
	std::string m_xdb_ud;
	boost::regex m_expression;
	size_t m_max_size;
	bool m_dont_strip;
	bool m_skip_folders;
	bool m_save_list;
};

class Packer
{
public:
	Packer(PackerOptions *options): m_options(options) {}
	~Packer();

	void process();
	void processFiles();

private:
	void init_archive();
	void process_folder(const std::string& path = "", const bool& dont_strip = false, const bool& skip_folders = false, const boost::regex& expression = boost::regex(""), const size_t& max_size = 0);
	void process_file(const std::string& path, const bool& dont_strip);
	void pack();

	std::string escape_json(const std::string &s);

	template <typename... Args> auto make_directory_range(bool recursive, Args... args);

	PackerOptions *m_options;

	xray_re::xr_writer *m_archive;
	std::string m_root;
	std::vector<std::string> m_folders;
	std::vector<xray_re::db_file*> m_files;
};
