// Copyright © 2025 CCP ehf.

#include "CreateResourceGroupFromFilterCliOperation.h"

#include <string>
#include <argparse/argparse.hpp>
#include <ResourceGroup.h>
#include <unordered_set>
#include <FilterIndexMappingFile.h>

CreateResourceGroupFromFilterCliOperation::CreateResourceGroupFromFilterCliOperation() :
	CliOperation( "create-group-from-filter", "Create filtered Resource Group(s)." ),
	m_filterIndexMappingFileId( "--filter-index-mapping-file" ),
	m_filterFileBasePathId( "--filter-file-basepath" ),
	m_resourceFileBasePathId( "--output-resource-file-basepath" ),
	m_documentVersionId( "--document-version" ),
	m_resourcePrefixId( "--resource-prefix" ),
	m_skipCompressionId( "--skip-compression" ),
	m_exportResourcesId( "--export-resources" ),
	m_exportResourcesDestinationTypeId( "--export-resources-destination-type" ),
	m_exportResourcesDestinationPathId( "--export-resources-destination-path" ),
	m_prefixMapBasepathId( "--prefix-map-basepath" ),
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

	AddArgument( m_filterIndexMappingFileId, "Path to filter index mapping file for resource filtering. See carbon-resources documentation for file specification.", true, true, "" );

    AddArgument( m_filterFileBasePathId, "Base path to filter files.", true, true, "" );

    AddArgument( m_resourceFileBasePathId, "Base path for output resource files.", false, true, "" );

	AddArgument( m_documentVersionId, "Document version for created resource group.", false, false, VersionToString( defaultImportParams.outputDocumentVersion ) );

	AddArgument( m_resourcePrefixId, R"(Optional resource path prefix, such as "res" or "app")", false, false, "" );

	AddArgumentFlag( m_skipCompressionId, "Set skip compression calculations on resources." );

	AddArgumentFlag( m_exportResourcesId, "Export resources after processing. see --export-resources-destination-type and --export-resources-destination-path" );

	AddArgument( m_exportResourcesDestinationTypeId, "Represents the type of repository where exported resources will be saved. Requires --export-resources", false, false, DestinationTypeToString( defaultImportParams.exportSettings.destinationSettings.destinationType ), ResourceDestinationTypeChoicesAsString() );

	AddArgument( m_exportResourcesDestinationPathId, "Represents the base path where the exported resources will be saved. Requires --export-resources", false, false, defaultImportParams.exportSettings.destinationSettings.basePath.string() );

	AddArgument( m_prefixMapBasepathId, "Base directory for prefix mappings defined in filter files.", false, false, "" );

	AddArgumentFlag( m_skipNonExistentInputDirectoriesId, "Skips input directories specified that don't exist rather than error." );

	AddArgument( m_streamChunkSizeId, "Represents the chunks streamed in bytes when streaming data.", false, false, SizeToString( defaultImportParams.fileStreamChunkSize ) );

	AddArgument( m_remoteUrlToGetCompressionId, "If supplied, url is checked to get compression information.", false, false, defaultImportParams.compressionCalculationSettings.remoteUrlToAttemptToGetCompression.string() );

	AddArgumentFlag( m_skipBinaryOperationCalculationId, "Set skip to skip binary operation for resources" );
}

bool CreateResourceGroupFromFilterCliOperation::Execute( std::string& returnErrorMessage ) const
{
	CarbonResources::CreateResourceGroupFromFilterParams createResourceGroupParams;

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

	bool versionIsValid = ParseDocumentVersion( m_argumentParser->get( m_documentVersionId ), createResourceGroupParams.outputDocumentVersion );

	if( !versionIsValid )
	{
		returnErrorMessage = "Invalid document version";

		return false;
	}

	createResourceGroupParams.resourcePrefix = m_argumentParser->get( m_resourcePrefixId );

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

    // Load filter settings from file
	std::filesystem::path basePathToFilterFiles = m_argumentParser->get<std::string>( m_filterFileBasePathId );

    std::filesystem::path basePathRespourceFiles = m_argumentParser->get<std::string>( m_resourceFileBasePathId );

	std::filesystem::path filterIndexMappingPath = m_argumentParser->get<std::string>( m_filterIndexMappingFileId );

    FilterIndexMappingFile filterIndexMappingFile;

    if (!filterIndexMappingFile.LoadFromFile(filterIndexMappingPath))
    {
		returnErrorMessage = "Failed to load filter index mappings from file provided: " + filterIndexMappingPath.string();

		return false;
    }

    // Set the filters from file data
	createResourceGroupParams.filterSettings.prefixMapBasePath = m_argumentParser->get<std::string>( m_prefixMapBasepathId );

    const std::vector<std::unique_ptr<FilterMapping>>& mappings = filterIndexMappingFile.GetFilterMappings();

    std::vector<std::unique_ptr<CarbonResources::ResourceGroupExportToFileParams>> exportParameters;

    for (auto& mapping : mappings)
    {
		std::unique_ptr<CarbonResources::Filter> filter = std::make_unique<CarbonResources::Filter>();

        for( auto filterPath : mapping->filterFilePaths )
        {
			filter->filterFilePaths.push_back( basePathToFilterFiles / filterPath );
        }

        createResourceGroupParams.filterSettings.filters.push_back( std::move( filter ) );

        std::unique_ptr<CarbonResources::ResourceGroupExportToFileParams> exportParamter = std::make_unique<CarbonResources::ResourceGroupExportToFileParams>();

        exportParamter->outputDocumentVersion = createResourceGroupParams.outputDocumentVersion;

        exportParamter->filename = basePathRespourceFiles / mapping->outputPath;

        exportParameters.push_back( std::move( exportParamter ) );

    }

	if( m_argumentParser->is_used( m_remoteUrlToGetCompressionId ) )
	{
		createResourceGroupParams.compressionCalculationSettings.remoteUrlToAttemptToGetCompression = m_argumentParser->get<std::string>( m_remoteUrlToGetCompressionId );
	}

	if( ShowCliStatusUpdates() )
	{
		PrintStartBanner( createResourceGroupParams, createResourceGroupParams.outputDocumentVersion, filterIndexMappingPath, basePathToFilterFiles, basePathRespourceFiles );
	}

	return CreateResourceGroups( createResourceGroupParams, exportParameters );
}

void CreateResourceGroupFromFilterCliOperation::PrintStartBanner(
	const CarbonResources::CreateResourceGroupFromFilterParams& createResourceGroupFromFilterParams,
	const CarbonResources::Version& outputVersion,
	const std::filesystem::path& filterIndexMappingFilePath,
    const std::filesystem::path& basePathToFilterFiles,
	const std::filesystem::path& basePathRespourceFiles ) const
{
	std::cout << "---Creating Resource Group---" << std::endl;

	PrintCommonOperationHeaderInformation();

	std::cout << "Output Document Version: " << VersionToString( outputVersion ) << std::endl;

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

	std::cout << "Resource filter index mapping file: " << filterIndexMappingFilePath << std::endl;

    std::cout << "Basepath to filter files: " << basePathToFilterFiles << std::endl;

    std::cout << "Basepath to resource files: " << basePathRespourceFiles << std::endl;

    std::cout << "Prefix basepath: " << createResourceGroupFromFilterParams.filterSettings.prefixMapBasePath << std::endl;

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

bool CreateResourceGroupFromFilterCliOperation::CreateResourceGroups(
	CarbonResources::CreateResourceGroupFromFilterParams& createResourceGroupFromFilterParams,
	std::vector<std::unique_ptr<CarbonResources::ResourceGroupExportToFileParams>>& exportParams ) const
{
	createResourceGroupFromFilterParams.callbackSettings.statusCallback = GetStatusCallback();
	createResourceGroupFromFilterParams.callbackSettings.verbosityLevel = GetVerbosityLevel();

    if( ShowCliStatusUpdates() )
	{
		CliStatusUpdate( "Creating resource group(s) from filter mappings." );
	}

    std::vector < std::unique_ptr<CarbonResources::ResourceGroup>> resourceGroups;

    for (auto& filter : createResourceGroupFromFilterParams.filterSettings.filters)
    {
		std::unique_ptr<CarbonResources::ResourceGroup> resourceGroup = std::make_unique<CarbonResources::ResourceGroup>();

		filter->outputResourceGroup = resourceGroup.get();

        resourceGroups.push_back( std::move( resourceGroup ) );
    }

	CarbonResources::Result createFromDirectoryResult = CarbonResources::ResourceGroup::CreateFromFilter( createResourceGroupFromFilterParams );

	if( createFromDirectoryResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( createFromDirectoryResult );

		return false;
	}


    // Export lists
	if( ShowCliStatusUpdates() )
	{
		CliStatusUpdate( "Exporting resource group to file." );
	}

	auto resourceIter = resourceGroups.begin();
	for( auto exportIter = exportParams.begin(); exportIter != exportParams.end(); exportIter++, resourceIter++ )
    {
		CarbonResources::ResourceGroupExportToFileParams& exportParams = **exportIter;

        exportParams.callbackSettings.statusCallback = GetStatusCallback();
		exportParams.callbackSettings.verbosityLevel = GetVerbosityLevel();

		CarbonResources::Result exportToFileResult = ( *resourceIter )->ExportToFile( exportParams );

        if( exportToFileResult.type != CarbonResources::ResultType::SUCCESS )
		{
			PrintCarbonResourcesError( exportToFileResult );

			return false;
		}
    }

	if( ShowCliStatusUpdates() )
	{
		CliStatusUpdate( "Operation complete." );
	}

	return true;
}