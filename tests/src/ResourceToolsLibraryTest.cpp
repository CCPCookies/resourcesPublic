#include <ResourceGroup.h>
#include <BundleResourceGroup.h>
#include <PatchResourceGroup.h>
#include <ResourceTools.h>

#include <gtest/gtest.h>

TEST( ResourceToolsTest, Md5ChecksumGeneration )
{
	std::string input = "Dummy";
	std::string output = "";

    EXPECT_TRUE( ResourceTools::GenerateMd5Checksum( input, output ) );

    EXPECT_EQ( output, "bcf036b6f33e182d4705f4f5b1af13ac" );
}

TEST( ResourceToolsTest, FowlerNollVoChecksumGeneration )
{
	std::string input = "res:/intromovie.txt";
	std::string output = "";

	EXPECT_TRUE( ResourceTools::GenerateFowlerNollVoChecksum( input, output ) );

	EXPECT_EQ( output, "a9d1721dd5cc6d54" );
}

TEST( ResourceToolsTest, DownloadFile )
{
	std::string url = "SomeUrl";
	std::string outputPath = "SomePath";

	EXPECT_TRUE( ResourceTools::DownloadFile( url, outputPath ) );

    // TODO check file exists and is correct using data checksum

}

TEST( ResourceToolsTest, GZipCompressData )
{
	std::string inputDataToCompress = "SomeData";

    std::string outputData = "";

	EXPECT_TRUE( ResourceTools::GZipCompressData( inputDataToCompress, outputData ) );

	// TODO check data using real data and data checksum
}

TEST( ResourceToolsTest, GZipUncompressData )
{
	std::string inputDataToUncompress = "SomeData";

	std::string outputData = "";

	EXPECT_TRUE( ResourceTools::GZipUncompressData( inputDataToUncompress, outputData ) );

	// TODO check data using real data and data checksum
}