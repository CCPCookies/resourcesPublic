// Copyright © 2025 CCP ehf.

#pragma once
#ifndef FilterIndexMappingFile_H
#define FilterIndexMappingFile_H

#include <filesystem>
#include <vector>

struct FilterMapping
{
	std::vector<std::filesystem::path> filterFilePaths;

	std::filesystem::path outputPath;
};

class FilterIndexMappingFile
{
public:
	FilterIndexMappingFile();

	~FilterIndexMappingFile( );

    bool LoadFromFile( const std::filesystem::path& path );

    const std::vector<std::unique_ptr<FilterMapping>>& GetFilterMappings() const;

private:

	std::vector<std::unique_ptr<FilterMapping>> m_filterMappings;

};

#endif // FilterIndexMappingFile_H