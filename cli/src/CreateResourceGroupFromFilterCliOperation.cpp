// Copyright © 2025 CCP ehf.

#include "CreateResourceGroupFromFilterCliOperation.h"

#include <string>
#include <argparse/argparse.hpp>
#include <ResourceGroup.h>
#include <unordered_set>

CreateResourceGroupFromFilterCliOperation::CreateResourceGroupFromFilterCliOperation() :
	CliOperation( "create-group-from-filter", "Create a Resource Group from a filter files." ),
	m_outputFileArgumentId( "--output-file" ),
	m_documentVersionArgumentId( "--document-version" ),
	m_resourcePrefixArgumentId( "--resource-prefix" ),
	m_skipCompressionId( "--skip-compression" ),
	m_exportResourcesId( "--export-resources" ),
	m_exportResourcesDestinationTypeId( "--export-resources-destination-type" ),
	m_exportResourcesDestinationPathId( "--export-resources-destination-path" ),
	m_filterFilesArgumentId( "--filter-file" ),
	m_filterFilesBasePathArgumentId( "--filter-file-basepath" ),
	m_skipNonExistentInputDirectoriesId( "--skip-non-existent-input-directories" ),
	m_streamChunkSizeId( "--stream-chunk-size" ),
	m_remoteUrlToGetCompressionId( "--remote-url-to-attempt-to-get-compression-info" ),
	m_skipBinaryOperationCalculationId( "--skip-binary-operation-calculation" )
{

	// Struct is inspected to ascertain default values
	// This keeps default value settings in one place
	// Lib defaults matches CLI
	CarbonResources::CreateResourceGroupFromFilterParams defaultImportParams;

	CarbonResources::ResourceGroupExportToFileParams defaultExportParams;

	AddArgument( m_outputFileArgumentId, "Filename for created resource group.", false, false, defaultExportParams.filename.string() );

	AddArgument( m_documentVersionArgumentId, "Document version for created resource group.", false, false, VersionToString( defaultImportParams.outputDocumentVersion ) );

	AddArgument( m_resourcePrefixArgumentId, R"(Optional resource path prefix, such as "res" or "app")", false, false, "" );

	AddArgumentFlag( m_skipCompressionId, "Set skip compression calculations on resources." );

	AddArgumentFlag( m_exportResourcesId, "Export resources after processing. see --export-resources-destination-type and --export-resources-destination-path" );

	AddArgument( m_exportResourcesDestinationTypeId, "Represents the type of repository where exported resources will be saved. Requires --export-resources", false, false, DestinationTypeToString( defaultImportParams.exportSettings.destinationSettings.destinationType ), ResourceDestinationTypeChoicesAsString() );

	AddArgument( m_exportResourcesDestinationPathId, "Represents the base path where the exported resources will be saved. Requires --export-resources", false, false, defaultImportParams.exportSettings.destinationSettings.basePath.string() );

	AddArgument( m_filterFilesArgumentId, "Path to filter file for resource filtering.", true, true, "" );

	AddArgument( m_filterFilesBasePathArgumentId, "Base directory for prefix mappings defined in filter files.", false, false, "" );

	AddArgumentFlag( m_skipNonExistentInputDirectoriesId, "Skips input directories specified that don't exist rather than error." );

	AddArgument( m_streamChunkSizeId, "Represents the chunks streamed in bytes when streaming data.", false, false, SizeToString( defaultImportParams.fileStreamChunkSize ) );

	AddArgument( m_remoteUrlToGetCompressionId, "If supplied, url is checked to get compression information.", false, false, defaultImportParams.compressionCalculationSettings.remoteUrlToAttemptToGetCompression.string() );

	AddArgumentFlag( m_skipBinaryOperationCalculationId, "Set skip to skip binary operation for resources" );
}

bool CreateResourceGroupFromFilterCliOperation::Execute( std::string& returnErrorMessage ) const
{
	CarbonResources::CreateResourceGroupFromFilterParams createResourceGroupParams;

	CarbonResources::ResourceGroupExportToFileParams exportParams;

	try
	{
		createResourceGroupParams.fileStreamChunkSize = std::stoull( m_argumentParser->get( m_streamChunkSizeId ) );
	}
	catch( std::invalid_argument& )
	{
		return false;
	}
	catch( std::out_of_range& )
	{
		return false;
	}

	bool versionIsValid = ParseDocumentVersion( m_argumentParser->get( m_documentVersionArgumentId ), createResourceGroupParams.outputDocumentVersion );

	if( !versionIsValid )
	{
		returnErrorMessage = "Invalid document version";

		return false;
	}

	createResourceGroupParams.resourcePrefix = m_argumentParser->get( m_resourcePrefixArgumentId );

	createResourceGroupParams.compressionCalculationSettings.calculateCompressions = !m_argumentParser->get<bool>( m_skipCompressionId );

	createResourceGroupParams.skipNonExistentInputDirectories = m_argumentParser->get<bool>( m_skipNonExistentInputDirectoriesId );

	createResourceGroupParams.calculateBinaryOperation = !m_argumentParser->get<bool>( m_skipBinaryOperationCalculationId );

	createResourceGroupParams.exportSettings.enabled = m_argumentParser->get<bool>( m_exportResourcesId );

	if( createResourceGroupParams.exportSettings.enabled )
	{
		std::string exportResourcesDesinationType = m_argumentParser->get<std::string>( m_exportResourcesDestinationTypeId );

		if( !StringToResourceDestinationType( exportResourcesDesinationType, createResourceGroupParams.exportSettings.destinationSettings.destinationType ) )
		{
			returnErrorMessage = "Invalid chunk destination type";

			return false;
		}

		createResourceGroupParams.exportSettings.destinationSettings.basePath = m_argumentParser->get<std::string>( m_exportResourcesDestinationPathId );
	}

	exportParams.filename = m_argumentParser->get<std::string>( m_outputFileArgumentId );

	exportParams.outputDocumentVersion = createResourceGroupParams.outputDocumentVersion;

	if( m_argumentParser->is_used( m_filterFilesArgumentId ) )
	{
		std::vector<std::filesystem::path> filterIniFilePaths;
		auto iniFileStringVector = m_argumentParser->get<std::vector<std::string>>( m_filterFilesArgumentId );

		for( const auto& iniPathStr : iniFileStringVector )
		{
			if( !iniPathStr.empty() )
			{
				createResourceGroupParams.filterSettings.filterFilePaths.emplace_back( iniPathStr );
			}
		}

		createResourceGroupParams.filterSettings.prefixMapBasePath = m_argumentParser->get<std::string>( m_filterFilesBasePathArgumentId );
	}

	if( m_argumentParser->is_used( m_remoteUrlToGetCompressionId ) )
	{
		createResourceGroupParams.compressionCalculationSettings.remoteUrlToAttemptToGetCompression = m_argumentParser->get<std::string>( m_remoteUrlToGetCompressionId );
	}

	if( ShowCliStatusUpdates() )
	{
		PrintStartBanner( createResourceGroupParams, exportParams );
	}

	return CreateResourceGroup( createResourceGroupParams, exportParams );
}

void CreateResourceGroupFromFilterCliOperation::PrintStartBanner(
	CarbonResources::CreateResourceGroupFromFilterParams& createResourceGroupFromFilterParams,
	CarbonResources::ResourceGroupExportToFileParams& ResourceGroupExportToFileParams ) const
{
	std::cout << "---Creating Resource Group---" << std::endl;

	PrintCommonOperationHeaderInformation();

	std::cout << "Output File: " << ResourceGroupExportToFileParams.filename << std::endl;

	std::cout << "Output Document Version: " << VersionToString( ResourceGroupExportToFileParams.outputDocumentVersion ) << std::endl;

	std::cout << "Resource Prefix: " << createResourceGroupFromFilterParams.resourcePrefix << std::endl;

	if( createResourceGroupFromFilterParams.compressionCalculationSettings.calculateCompressions )
	{
		std::cout << "Calculate Compression: On" << std::endl;
	}
	else
	{
		std::cout << "Calculate Compression: Off" << std::endl;
	}

	if( createResourceGroupFromFilterParams.exportSettings.enabled )
	{
		std::cout << "Export Resources: On" << std::endl;

		std::cout << "Export Resources Type: " << DestinationTypeToString( createResourceGroupFromFilterParams.exportSettings.destinationSettings.destinationType ) << std::endl;

		std::cout << "Export Resources Base Path: " << createResourceGroupFromFilterParams.exportSettings.destinationSettings.basePath << std::endl;
	}
	else
	{
		std::cout << "Export Resources: Off" << std::endl;
	}

	if( !createResourceGroupFromFilterParams.filterSettings.filterFilePaths.empty() )
	{
		std::cout << "Resource Filter INI File(s): " << std::endl;

		for( const auto& iniPath : createResourceGroupFromFilterParams.filterSettings.filterFilePaths )
		{
			std::cout << " - " << iniPath.generic_string() << std::endl;
		}

		std::cout << "Filter prefix base path: " << createResourceGroupFromFilterParams.filterSettings.prefixMapBasePath << std::endl;
	}
	else
	{
		std::cout << "Resource Filter INI File(s): None" << std::endl;
	}

	if( createResourceGroupFromFilterParams.skipNonExistentInputDirectories )
	{
		std::cout << "Skip non existant input directories: On" << std::endl;
	}
	else
	{
		std::cout << "Skip non existant input directories: Off" << std::endl;
	}

	if( createResourceGroupFromFilterParams.calculateBinaryOperation )
	{
		std::cout << "Binary operation calculation: On" << std::endl;
	}
	else
	{
		std::cout << "Binary operation calculation: Off" << std::endl;
	}

	if( createResourceGroupFromFilterParams.compressionCalculationSettings.remoteUrlToAttemptToGetCompression != "" )
	{
		std::cout << "Compression info check url: " << createResourceGroupFromFilterParams.compressionCalculationSettings.remoteUrlToAttemptToGetCompression << std::endl;
	}

	std::cout << "File stream chunk Size: " << createResourceGroupFromFilterParams.fileStreamChunkSize << " Bytes" << std::endl;

	std::cout << "----------------------------\n"
			  << std::endl;
}

bool CreateResourceGroupFromFilterCliOperation::CreateResourceGroup(
	CarbonResources::CreateResourceGroupFromFilterParams& createResourceGroupFromFilterParams,
	CarbonResources::ResourceGroupExportToFileParams& ResourceGroupExportToFileParams ) const
{
	CarbonResources::ResourceGroup resourceGroup;

	createResourceGroupFromFilterParams.callbackSettings.statusCallback = GetStatusCallback();
	createResourceGroupFromFilterParams.callbackSettings.verbosityLevel = GetVerbosityLevel();

	if( ShowCliStatusUpdates() )
	{
		CliStatusUpdate( "Creating resource group from directory." );
	}

	CarbonResources::Result createFromDirectoryResult = resourceGroup.CreateFromFilter( createResourceGroupFromFilterParams );

	if( createFromDirectoryResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( createFromDirectoryResult );

		return false;
	}

	ResourceGroupExportToFileParams.callbackSettings.statusCallback = GetStatusCallback();
	ResourceGroupExportToFileParams.callbackSettings.verbosityLevel = GetVerbosityLevel();

	if( ShowCliStatusUpdates() )
	{
		CliStatusUpdate( "Exporting resource group to file." );
	}

	CarbonResources::Result exportToFileResult = resourceGroup.ExportToFile( ResourceGroupExportToFileParams );

	if( exportToFileResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( exportToFileResult );

		return false;
	}

	if( ShowCliStatusUpdates() )
	{
		CliStatusUpdate( "Operation complete." );
	}

	return true;
}