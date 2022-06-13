#pragma once

#include "xray_re/xr_types.hxx"

#include <boost/regex.hpp>
#include <string>
#include <vector>

class DBTools
{
public:
	virtual ~DBTools() = default;

	static void unpack(const std::string& source_path, const std::string& destination_path, const xray_re::DBVersion& version, const std::string& filter);
	static void pack(const std::string& source_path, const std::string& destination_path, const xray_re::DBVersion& version, const std::string& xdb_ud, const bool& dont_strip, const bool& skip_folders, boost::regex& expression);
	static void packFiles(const std::vector<std::string>& files, const std::string& destination_path, const xray_re::DBVersion& version, const std::string& xdb_ud, const bool& dont_strip, const bool& skip_folders);

	static bool is_db(const std::string& extension);
	static bool is_xdb(const std::string& extension);
	static bool is_xrp(const std::string& extension);
	static bool is_xp(const std::string& extension);
	static bool is_known(const std::string& extension);

	static void set_debug(const bool value);
};
