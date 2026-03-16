// Copyright © 2025 CCP ehf.

#pragma once
#ifndef ResourceFilter_H
#define ResourceFilter_H

#include "FilterFileReader.h"

#include <filesystem>
#include <vector>
#include <map>
#include <regex>

namespace ResourceTools
{

struct FilterPath
{
	std::string sectionId;

	std::string prefixId;

	std::string path;

	std::string matchPattern;

	std::regex matchPatternRegex;

	std::set<std::string> includeRules;

	std::set<std::string> excludeRules;

	bool containsLocalIncludeExcludeRules;
};

class ResourceFilter
{
public:
	ResourceFilter() = default;

	~ResourceFilter() = default;

	bool SetFromFilterFileData( const FilterFile& fileData );

	bool CheckPath( const std::filesystem::path& path ) const;

	bool CheckPath( const std::filesystem::path& path, std::string& matchSectionId, std::string& matchPath ) const;

	const std::vector<std::filesystem::path>& GetPrefixPaths() const;

private:
	void ConvertResPathToPattern( std::string resPath, std::string& pattern ) const;

private:
	std::map<std::string,std::vector<std::unique_ptr<FilterPath>>> m_paths;

	std::vector<std::filesystem::path> m_prefixPaths;
};
}

#endif //ResourceFilter_H