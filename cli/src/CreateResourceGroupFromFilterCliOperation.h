// Copyright © 2025 CCP ehf.

#pragma once
#ifndef CreateResourceGroupFromFilterCliOperation_H
#define CreateResourceGroupFromFilterCliOperation_H

#include <filesystem>

#include "CliOperation.h"

#include <ResourceGroup.h>

class CreateResourceGroupFromFilterCliOperation : public CliOperation
{
public:
	CreateResourceGroupFromFilterCliOperation();

	virtual bool Execute( std::string& returnErrorMessage ) const final;

private:
	void PrintStartBanner(
		const CarbonResources::CreateResourceGroupFromFilterParams& createResourceGroupFromFilterParams,
		const CarbonResources::Version& outputVersion,
		const std::filesystem::path& filterIndexMappingFilePath,
		const std::filesystem::path& basePathToFilterFiles,
		const std::filesystem::path& basePathRespourceFiles ) const;

	bool CreateResourceGroups(
		CarbonResources::CreateResourceGroupFromFilterParams& createResourceGroupFromFilterParams,
		std::vector<std::unique_ptr<CarbonResources::ResourceGroupExportToFileParams>>& exportParams ) const;

private:

    std::string m_filterIndexMappingFileId;

    std::string m_filterFileBasePathId;

    std::string m_resourceFileBasePathId;

	std::string m_documentVersionId;

	std::string m_resourcePrefixId;

	std::string m_skipCompressionId;

	std::string m_exportResourcesId;

	std::string m_exportResourcesDestinationTypeId;

	std::string m_exportResourcesDestinationPathId;

	std::string m_prefixMapBasepathId;

	std::string m_skipNonExistentInputDirectoriesId;

	std::string m_streamChunkSizeId;

	std::string m_remoteUrlToGetCompressionId;

	std::string m_skipBinaryOperationCalculationId;
};

#endif // CreateResourceGroupFromFilterCliOperation_H