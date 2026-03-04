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
		CarbonResources::CreateResourceGroupFromFilterParams& createResourceGroupFromFilterParams,
		CarbonResources::ResourceGroupExportToFileParams& ResourceGroupExportToFileParams ) const;

	bool CreateResourceGroup(
		CarbonResources::CreateResourceGroupFromFilterParams& createResourceGroupFromFilterParams,
		CarbonResources::ResourceGroupExportToFileParams& ResourceGroupExportToFileParams ) const;

private:
	std::string m_outputFileArgumentId;

	std::string m_documentVersionArgumentId;

	std::string m_resourcePrefixArgumentId;

	std::string m_skipCompressionId;

	std::string m_exportResourcesId;

	std::string m_exportResourcesDestinationTypeId;

	std::string m_exportResourcesDestinationPathId;

	std::string m_filterFilesArgumentId;

	std::string m_filterFilesBasePathArgumentId;

	std::string m_skipNonExistentInputDirectoriesId;

	std::string m_streamChunkSizeId;

	std::string m_remoteUrlToGetCompressionId;

	std::string m_skipBinaryOperationCalculationId;
};

#endif // CreateResourceGroupFromFilterCliOperation_H