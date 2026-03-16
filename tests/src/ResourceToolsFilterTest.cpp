// Copyright © 2025 CCP ehf.

#include "ResourceToolsTest.h"

#include "FilterFileReader.h"
#include "ResourceFilter.h"

void LoadResourceFilterFromIniFileData(const std::string& iniData, ResourceTools::ResourceFilter& resourceFilter)
{
	try
	{
		ResourceTools::FilterFile fileData;

		ResourceTools::FilterFileReader::LoadFromIniFileData( iniData.data(), iniData.size(), fileData );

        EXPECT_TRUE(resourceFilter.SetFromFilterFileData( fileData ));
	}
	catch( const std::exception& e )
	{
		std::string errorMsg = "Failed to read filter file: " + std::string( e.what() );

		FAIL() << errorMsg;
	}
}

TEST_F(ResourceToolsTest, Filtering_LoadValidFilterFile)
{
	std::string iniFile = "[DEFAULT]\n"
						  "prefixmap = prefix1:.\n"
                          "[testSection]\n"
                          "filter = [.type1] ![.type2]\n"
                          "respaths = prefix1:/* [.type3] ![.type4]";

    ResourceTools::FilterFile fileData;

    try
    {
		ResourceTools::FilterFileReader::LoadFromIniFileData( iniFile.data(), iniFile.size(), fileData );
    }
	catch( const std::exception& e )
	{
		std::string errorMsg = "Failed to read filter file: " + std::string( e.what() );

		FAIL() << errorMsg;
	
	}

    ASSERT_NE( fileData.prefixes.size(), 0 );

    std::shared_ptr<ResourceTools::Prefix> prefix = *fileData.prefixes.begin();

    EXPECT_EQ( prefix->id, "prefix1" );

    ASSERT_EQ( prefix->paths.size(), 1 );

    EXPECT_EQ( prefix->paths.at( 0 ), "." );

    ASSERT_EQ( fileData.filterSections.size(), 1 );

    //Note: INI reader lib is turning all section headers to lowercase 
    EXPECT_EQ( fileData.filterSections.at(0)->id, "testsection" );  
	
    ASSERT_EQ( fileData.filterSections.at( 0 )->includeRules.size(), 1 );
	
    EXPECT_EQ( *fileData.filterSections.at( 0 )->includeRules.begin(), ".type1" );
	
    ASSERT_EQ( fileData.filterSections.at( 0 )->excludeRules.size(), 1 );
	
	EXPECT_EQ( *fileData.filterSections.at( 0 )->excludeRules.begin(), ".type2" );
	
    ASSERT_EQ( fileData.filterSections.at( 0 )->respaths.size(), 1 );

    EXPECT_EQ( fileData.filterSections.at( 0 )->respaths.at( 0 )->prefix->id, "prefix1" );

    ASSERT_EQ( fileData.filterSections.at( 0 )->respaths.at( 0 )->path, "/*" );

    ASSERT_EQ( fileData.filterSections.at( 0 )->respaths.at( 0 )->includeRules.size(), 1 );

	EXPECT_EQ( *fileData.filterSections.at( 0 )->respaths.at( 0 )->includeRules.begin(), ".type3" );

	ASSERT_EQ( fileData.filterSections.at( 0 )->respaths.at( 0 )->excludeRules.size(), 1 );

	EXPECT_EQ( *fileData.filterSections.at( 0 )->respaths.at( 0 )->excludeRules.begin(), ".type4" );

}
TEST_F( ResourceToolsTest, Filtering_AddFilterFile )
{
	std::string iniFile = "[DEFAULT]\n"
						  "prefixmap = prefix1:.\n"
						  "[testSection]\n"
						  "filter = [.type1] ![.type2]\n"
						  "respaths = prefix1:/* [.type1]![type2]";

    ResourceTools::FilterFile fileData;

	try
	{
		ResourceTools::FilterFileReader::LoadFromIniFileData( iniFile.data(), iniFile.size(), fileData );
	}
	catch( const std::exception& e )
	{
		std::string errorMsg = "Failed to read filter file: " + std::string( e.what() );

		FAIL() << errorMsg;
	}


    ResourceTools::ResourceFilter resourceFilter;

    ASSERT_TRUE( resourceFilter.SetFromFilterFileData( fileData ) );
}
TEST_F( ResourceToolsTest, Filtering_AsteriskPatternMatch )
{
	std::string iniFile = "[DEFAULT]\n"
						  "prefixmap = prefix1:.\n"
						  "[testSection]\n"
						  "respaths = prefix1:/*";


    ResourceTools::ResourceFilter resourceFilter;

    LoadResourceFilterFromIniFileData( iniFile, resourceFilter );

    std::filesystem::path validPath = "File";

    ASSERT_TRUE( resourceFilter.CheckPath( validPath ) );

}

TEST_F( ResourceToolsTest, Filtering_AsteriskPatternMissmatch )
{
	std::string iniFile = "[DEFAULT]\n"
						  "prefixmap = prefix1:.\n"
						  "[testSection]\n"
						  "respaths = prefix1:/*";



    ResourceTools::ResourceFilter resourceFilter;

	LoadResourceFilterFromIniFileData( iniFile, resourceFilter );

	std::filesystem::path validPath = "Subfolder/File";

	ASSERT_FALSE( resourceFilter.CheckPath( validPath ) );
}

TEST_F( ResourceToolsTest, Filtering_ElipsisPatternMatch )
{
	std::string iniFile = "[DEFAULT]\n"
						  "prefixmap = prefix1:.\n"
						  "[testSection]\n"
						  "respaths = prefix1:/...";


    ResourceTools::ResourceFilter resourceFilter;

	LoadResourceFilterFromIniFileData( iniFile, resourceFilter );

    std::vector<std::filesystem::path> validPaths = {
		"File",
		"Subfolder1/File",
		"Subfolder2/File"
	};

    for (auto path : validPaths)
	{
		ASSERT_TRUE( resourceFilter.CheckPath( path ) );
    }
	
}

TEST_F( ResourceToolsTest, Filtering_SpecificFile )
{
	std::string iniFile = "[DEFAULT]\n"
						  "prefixmap = prefix1:.\n"
						  "[testSection]\n"
						  "respaths = prefix1:/File";

    ResourceTools::ResourceFilter resourceFilter;

	LoadResourceFilterFromIniFileData( iniFile, resourceFilter );

	std::filesystem::path validPath = "File";

	ASSERT_TRUE( resourceFilter.CheckPath( validPath ) );

    std::filesystem::path invalidPath = "NonMatching";

    ASSERT_FALSE( resourceFilter.CheckPath( invalidPath ) );
}

TEST_F( ResourceToolsTest, Filtering_FilterIncludeRule )
{
	std::string iniFile = "[DEFAULT]\n"
						  "prefixmap = prefix1:.\n"
						  "[testSection]\n"
						  "filter = [ .type1 ]\n"
						  "respaths = prefix1:/*";


	ResourceTools::ResourceFilter resourceFilter;

	LoadResourceFilterFromIniFileData( iniFile, resourceFilter );

	std::filesystem::path validPath = "File.type1";

	ASSERT_TRUE( resourceFilter.CheckPath( validPath ) );

    std::filesystem::path invalidPath = "File.type2";

    ASSERT_FALSE( resourceFilter.CheckPath( invalidPath ) );
}

TEST_F( ResourceToolsTest, Filtering_FilterExcludeRule )
{
	std::string iniFile = "[DEFAULT]\n"
						  "prefixmap = prefix1:.\n"
						  "[testSection]\n"
						  "filter = ![ .type1 ]\n"
						  "respaths = prefix1:/*";


	ResourceTools::ResourceFilter resourceFilter;

	LoadResourceFilterFromIniFileData( iniFile, resourceFilter );

    std::filesystem::path validPath = "File.type2";

	ASSERT_TRUE( resourceFilter.CheckPath( validPath ) );

	std::filesystem::path invalidPath = "File.type1";

	ASSERT_FALSE( resourceFilter.CheckPath( invalidPath ) );
	
}

TEST_F( ResourceToolsTest, Filtering_FilterOverlappingIncludeExcludeRule1 )
{
	std::string iniFile = "[DEFAULT]\n"
						  "prefixmap = prefix1:.\n"
						  "[testSection]\n"
						  "filter = [ .type1 ]![ .type1 ]\n"
						  "respaths = prefix1:/*";


	ResourceTools::ResourceFilter resourceFilter;

	LoadResourceFilterFromIniFileData( iniFile, resourceFilter );

	std::filesystem::path invalidPath = "File.type1";

	ASSERT_FALSE( resourceFilter.CheckPath( invalidPath ) );

    std::filesystem::path invalidPath2 = "File.type2";

	ASSERT_FALSE( resourceFilter.CheckPath( invalidPath2 ) );

}

TEST_F( ResourceToolsTest, Filtering_FilterOverlappingIncludeExcludeRule2 )
{
	std::string iniFile = "[DEFAULT]\n"
						  "prefixmap = prefix1:.\n"
						  "[testSection]\n"
						  "filter = ![ .type1 ][ .type1 ]\n"
						  "respaths = prefix1:/*";


	ResourceTools::ResourceFilter resourceFilter;

	LoadResourceFilterFromIniFileData( iniFile, resourceFilter );

	std::filesystem::path invalidPath = "File.type1";

	ASSERT_FALSE( resourceFilter.CheckPath( invalidPath ) );

    std::filesystem::path invalidPath2 = "File.type2";

	ASSERT_FALSE( resourceFilter.CheckPath( invalidPath2 ) );
}

TEST_F( ResourceToolsTest, Filtering_FilterRespathExcludeRule )
{
	std::string iniFile = "[DEFAULT]\n"
						  "prefixmap = prefix1:.\n"
						  "[testSection]\n"
						  "respaths = prefix1:/* ![ File ]";


	ResourceTools::ResourceFilter resourceFilter;

	LoadResourceFilterFromIniFileData( iniFile, resourceFilter );

	std::filesystem::path invalidPath = "File";

	ASSERT_FALSE( resourceFilter.CheckPath( invalidPath ) );

    std::filesystem::path validPath = "Another";

	ASSERT_TRUE( resourceFilter.CheckPath( validPath ) );
}

TEST_F( ResourceToolsTest, Filtering_FilterRespathIncludeRule )
{
	std::string iniFile = "[DEFAULT]\n"
						  "prefixmap = prefix1:.\n"
						  "[testSection]\n"
						  "respaths = prefix1:/* [ File ]";


	ResourceTools::ResourceFilter resourceFilter;

	LoadResourceFilterFromIniFileData( iniFile, resourceFilter );

	std::filesystem::path validPath = "File";

	ASSERT_TRUE( resourceFilter.CheckPath( validPath ) );

    std::filesystem::path invalidPath = "Another";

	ASSERT_FALSE( resourceFilter.CheckPath( invalidPath ) );
}

TEST_F( ResourceToolsTest, Filtering_FilterIncludeRuleWithOverlappingRespathExcludeRule )
{
	std::string iniFile = "[DEFAULT]\n"
						  "prefixmap = prefix1:.\n"
						  "[testSection]\n"
						  "filter = [ File ]\n"
						  "respaths = prefix1:/* ![ File ]";


	ResourceTools::ResourceFilter resourceFilter;

	LoadResourceFilterFromIniFileData( iniFile, resourceFilter );

	std::filesystem::path invalidPath = "File";

	ASSERT_FALSE( resourceFilter.CheckPath( invalidPath ) );

    std::filesystem::path invalidPath2 = "Another";

	ASSERT_FALSE( resourceFilter.CheckPath( invalidPath2 ) );
}

TEST_F( ResourceToolsTest, Filtering_FilterExcludeRuleWithOverlappingRespathIncludeRule )
{
	std::string iniFile = "[DEFAULT]\n"
						  "prefixmap = prefix1:.\n"
						  "[testSection]\n"
						  "filter = ![ File ]\n"
						  "respaths = prefix1:/* [ File ]";


	ResourceTools::ResourceFilter resourceFilter;

	LoadResourceFilterFromIniFileData( iniFile, resourceFilter );

	std::filesystem::path invalidPath = "File";

	ASSERT_FALSE( resourceFilter.CheckPath( invalidPath ) );

    std::filesystem::path invalidPath2 = "Another";

	ASSERT_FALSE( resourceFilter.CheckPath( invalidPath2 ) );
}

TEST_F( ResourceToolsTest, Filtering_MultiPrefixMatch )
{
	std::string iniFile = "[DEFAULT]\n"
						  "prefixmap = prefix1:Path1 prefix2:Path2\n"
						  "[testSection]\n"
						  "respaths = prefix1:/*\n"
						  "           prefix2:/*";


	ResourceTools::ResourceFilter resourceFilter;

	LoadResourceFilterFromIniFileData( iniFile, resourceFilter );

	std::filesystem::path prefix1ValidPath = "Path1/File";

	ASSERT_TRUE( resourceFilter.CheckPath( prefix1ValidPath ) );

    std::filesystem::path prefix2ValidPath = "Path2/File";

    ASSERT_TRUE( resourceFilter.CheckPath( prefix2ValidPath ) );
}

TEST_F( ResourceToolsTest, Filtering_MultiPrefixExcludeAppliesAcrossBoth )
{
	std::string iniFile = "[DEFAULT]\n"
						  "prefixmap = prefix1:Path1 prefix2:Path2\n"
						  "[testSection]\n"
						  "filter = ![ File ]\n"
						  "respaths = prefix1:/*\n"
						  "           prefix2:/*";


	ResourceTools::ResourceFilter resourceFilter;

	LoadResourceFilterFromIniFileData( iniFile, resourceFilter );

	std::filesystem::path prefix1InvalidPath = "Path1/File";

	ASSERT_FALSE( resourceFilter.CheckPath( prefix1InvalidPath ) );

	std::filesystem::path prefix2InvalidPath = "Path2/File";

	ASSERT_FALSE( resourceFilter.CheckPath( prefix2InvalidPath ) );

    std::filesystem::path prefix1ValidPath = "Path1/Another";

	ASSERT_TRUE( resourceFilter.CheckPath( prefix1ValidPath ) );

	std::filesystem::path prefix2ValidPath = "Path2/Another";

	ASSERT_TRUE( resourceFilter.CheckPath( prefix2ValidPath ) );
}

TEST_F( ResourceToolsTest, Filtering_MultiPrefixIncludeAppliesAcrossBoth )
{
	std::string iniFile = "[DEFAULT]\n"
						  "prefixmap = prefix1:Path1 prefix2:Path2\n"
						  "[testSection]\n"
						  "filter = [ File ]\n"
						  "respaths = prefix1:/*\n"
						  "           prefix2:/*";


	ResourceTools::ResourceFilter resourceFilter;

	LoadResourceFilterFromIniFileData( iniFile, resourceFilter );

	std::filesystem::path prefix1ValidPath = "Path1/File";

	ASSERT_TRUE( resourceFilter.CheckPath( prefix1ValidPath ) );

	std::filesystem::path prefix2ValidPath = "Path2/File";

	ASSERT_TRUE( resourceFilter.CheckPath( prefix2ValidPath ) );

    std::filesystem::path prefix1InvalidPath = "Path1/Another";

	ASSERT_FALSE( resourceFilter.CheckPath( prefix1InvalidPath ) );

	std::filesystem::path prefix2inValidPath = "Path2/Another";

	ASSERT_FALSE( resourceFilter.CheckPath( prefix2inValidPath ) );
}

TEST_F( ResourceToolsTest, Filtering_MultiPrefixFilterIncludeOverrideExcludeAppliesAccrossBoth )
{
	std::string iniFile = "[DEFAULT]\n"
						  "prefixmap = prefix1:Path1 prefix2:Path2\n"
						  "[testSection]\n"
						  "filter = [ File ]\n"
						  "respaths = prefix1:/* ![ File ]\n"
						  "           prefix2:/*";


	ResourceTools::ResourceFilter resourceFilter;

	LoadResourceFilterFromIniFileData( iniFile, resourceFilter );

	std::filesystem::path prefix1InvalidPath = "Path1/File";

	ASSERT_FALSE( resourceFilter.CheckPath( prefix1InvalidPath ) );

	std::filesystem::path prefix2InvalidPath = "Path2/File";

	ASSERT_FALSE( resourceFilter.CheckPath( prefix2InvalidPath ) );

    std::filesystem::path prefix1InvalidPath2 = "Path1/Another";

	ASSERT_FALSE( resourceFilter.CheckPath( prefix1InvalidPath2 ) );

	std::filesystem::path prefix2InvalidPath2 = "Path2/Another";

	ASSERT_FALSE( resourceFilter.CheckPath( prefix2InvalidPath2 ) );
}

TEST_F( ResourceToolsTest, Filtering_MultiPrefixFilterIncludeAddedPathIncludeAppliesToLaterPaths )
{
	std::string iniFile = "[DEFAULT]\n"
						  "prefixmap = prefix1:Path1 prefix2:Path2\n"
						  "[testSection]\n"
						  "filter = [ .type1 ]\n"
						  "respaths = prefix1:/* [ .type2 ]\n"
						  "           prefix2:/*";


	ResourceTools::ResourceFilter resourceFilter;

	LoadResourceFilterFromIniFileData( iniFile, resourceFilter );

	std::filesystem::path prefix1ValidPath = "Path1/File.type1";

	ASSERT_TRUE( resourceFilter.CheckPath( prefix1ValidPath ) );

	std::filesystem::path prefix2ValidPath = "Path2/File.type2";

	ASSERT_TRUE( resourceFilter.CheckPath( prefix2ValidPath ) );
}

TEST_F( ResourceToolsTest, Filtering_MultiPrefixFilterIncludeWithLaterRespathExclude )
{
	std::string iniFile = "[DEFAULT]\n"
						  "prefixmap = prefix1:Path1 prefix2:Path2\n"
						  "[testSection]\n"
						  "filter = [ .type1 ]\n"
						  "respaths = prefix1:/*\n"
						  "           prefix1:/* ![ .type1 ]\n"
						  "           prefix2:/*";


	ResourceTools::ResourceFilter resourceFilter;

	LoadResourceFilterFromIniFileData( iniFile, resourceFilter );

	std::filesystem::path prefix1ValidPath = "Path1/File.type1";

	ASSERT_TRUE( resourceFilter.CheckPath( prefix1ValidPath ) );

	std::filesystem::path prefix2InvalidPath = "Path2/File.type1";

	ASSERT_FALSE( resourceFilter.CheckPath( prefix2InvalidPath ) );
}

TEST_F( ResourceToolsTest, Filtering_SpecificFileWithInclude )
{
    // Surprisingly this should result in not a match
    // A warning would probably be useful
	std::string iniFile = "[DEFAULT]\n"
						  "prefixmap = prefix1:.\n"
						  "[testSection]\n"
						  "respaths = prefix1:/File [ File ]";

    ResourceTools::ResourceFilter resourceFilter;

	LoadResourceFilterFromIniFileData( iniFile, resourceFilter );

	std::filesystem::path invalidPath = "File";

	ASSERT_FALSE( resourceFilter.CheckPath( invalidPath ) );
}

TEST_F( ResourceToolsTest, Filtering_SpecificFileWithNonMatchingExclude )
{
	// Surprisingly this should result in not a match
	// A warning would probably be useful
	std::string iniFile = "[DEFAULT]\n"
						  "prefixmap = prefix1:.\n"
						  "[testSection]\n"
						  "respaths = prefix1:/File ![ NONMatch ]\n"
	                      "           prefix1:/File2";


	ResourceTools::ResourceFilter resourceFilter;

	LoadResourceFilterFromIniFileData( iniFile, resourceFilter );

	std::filesystem::path invalidPath = "File";

	ASSERT_FALSE( resourceFilter.CheckPath( invalidPath ) );

    std::filesystem::path validPath = "File2";

	ASSERT_TRUE( resourceFilter.CheckPath( validPath ) );
}

TEST_F( ResourceToolsTest, Filtering_SpecificFileWithOverlappingExcludeRule )
{
	std::string iniFile = "[DEFAULT]\n"
						  "prefixmap = prefix1:.\n"
						  "[testSection]\n"
						  "filter = ![ File ]\n"
						  "respaths = prefix1:/File";


	ResourceTools::ResourceFilter resourceFilter;

	LoadResourceFilterFromIniFileData( iniFile, resourceFilter );

	std::filesystem::path validPath = "File";

	ASSERT_TRUE( resourceFilter.CheckPath( validPath ) );
}

TEST_F( ResourceToolsTest, Filtering_FilterGlobalIncludeRule )
{
	std::string iniFile = "[DEFAULT]\n"
						  "prefixmap = prefix1:.\n"
						  "filter = [ .type1 ]\n"
						  "[testSection]\n"
						  "respaths = prefix1:/*";


	ResourceTools::ResourceFilter resourceFilter;

	LoadResourceFilterFromIniFileData( iniFile, resourceFilter );

	std::filesystem::path validPath = "File.type1";

	ASSERT_TRUE( resourceFilter.CheckPath( validPath ) );

	std::filesystem::path invalidPath = "File.type2";

	ASSERT_FALSE( resourceFilter.CheckPath( invalidPath ) );
}