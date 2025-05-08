#include "CreateResourceGroupCliOperation.h"

#include <string>
#include <argparse/argparse.hpp>
#include <ResourceGroup.h>

CreateResourceGroupCliOperation::CreateResourceGroupCliOperation() :
	CliOperation( "create-group", "Create a resource group from a given directory." ),
	m_createResourceGroupPathArgumentId( "input-directory" ),
	m_createResourceGroupOutputFilenameArgumentId( "--output-filename" )
{

	AddRequiredPositionalArgument( m_createResourceGroupPathArgumentId, "Base directory to create resource group from." );

	AddArgument( m_createResourceGroupOutputFilenameArgumentId, "Filename for created resource group.", true, "ResourceGroup.yaml" );
}

bool CreateResourceGroupCliOperation::Execute() const
{
	std::string inputDirectory = m_argumentParser->get<std::string>( m_createResourceGroupPathArgumentId );

	std::string resourceGroupOutputDirectory = m_argumentParser->get<std::string>( m_createResourceGroupOutputFilenameArgumentId );

	return CreateResourceGroup( inputDirectory, resourceGroupOutputDirectory );
}

bool CreateResourceGroupCliOperation::CreateResourceGroup( const std::filesystem::path& inputDirectory, const std::filesystem::path& resourceGroupOutputDirectory ) const
{
	CarbonResources::ResourceGroup resourceGroup;

	CarbonResources::CreateResourceGroupFromDirectoryParams createResourceGroupParams;

	createResourceGroupParams.directory = inputDirectory;

	createResourceGroupParams.statusCallback = GetStatusCallback();

	CarbonResources::Result createFromDirectoryResult = resourceGroup.CreateFromDirectory( createResourceGroupParams );

	if( createFromDirectoryResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( createFromDirectoryResult );

		return false;
	}

	CarbonResources::ResourceGroupExportToFileParams exportParams;

	exportParams.filename = resourceGroupOutputDirectory;

	exportParams.statusCallback = GetStatusCallback();

	CarbonResources::Result exportToFileResult = resourceGroup.ExportToFile( exportParams );

	if( exportToFileResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( exportToFileResult );

		return false;
	}

	return true;
}