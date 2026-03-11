// Copyright © 2025 CCP ehf.

#pragma once
#ifndef FilterFileReader_H
#define FilterFileReader_H

#include <filesystem>
#include <vector>
#include <unordered_map>
#include <set>
#include <memory>

namespace ResourceTools
{

struct Prefix
{
	std::string id;

	std::vector<std::filesystem::path> paths;
};

struct ResPath
{
	std::shared_ptr<Prefix> prefix;

	std::filesystem::path path = "";

	std::set<std::string> includeRules;

	std::set<std::string> excludeRules;
};

struct FilterSection
{
	std::string id;

	std::set<std::string> includeRules;

	std::set<std::string> excludeRules;

	std::vector<std::unique_ptr<ResPath>> respaths;

    bool containsNonWildcardResPath;
};

struct FilterFile
{
	std::vector<std::shared_ptr<Prefix>> prefixes;

	std::set<std::string> includeRules;

	std::set<std::string> excludeRules;

	std::vector<std::unique_ptr<FilterSection>> filterSections;
};

class FilterFileReader
{
public:
	FilterFileReader();

	~FilterFileReader();

	static void LoadFromIniFileData( const char* data, size_t dataSize, FilterFile& fileData, bool ignoreCase = false );

private:
	static void ParsePrefixMappings( const std::string& prefixStr, std::vector<std::shared_ptr<Prefix>>& prefixes );

	static void ParsePrefixPaths( const std::string& prefixPathsStr, std::vector<std::filesystem::path>& paths );

	static void ParseIncludeExcludeRules( const std::string& rulesStr, std::set<std::string>& includeRules, std::set<std::string>& excludeRules, bool ignoreCase );

	static void ParseSectionResPathEntry( const std::string& filterStr, std::vector<std::unique_ptr<ResPath>>& resPaths, std::vector<std::shared_ptr<Prefix>>& prefixes, bool ignoreCase );
};
}

#endif //FilterFileReader_H