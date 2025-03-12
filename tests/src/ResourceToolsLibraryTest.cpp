#include <ResourceGroup.h>
#include <BundleResourceGroup.h>
#include <PatchResourceGroup.h>
#include <ResourceTools.h>

#include <gtest/gtest.h>

#include "CarbonResourcesTestFixture.h"

struct ResourceToolsTest : public CarbonResourcesTestFixture
{
};

TEST_F( ResourceToolsTest, Md5ChecksumGeneration )
{
	std::string input = "Dummy";
	std::string output = "";

    EXPECT_TRUE( ResourceTools::GenerateMd5Checksum( input, output ) );

    EXPECT_EQ( output, "bcf036b6f33e182d4705f4f5b1af13ac" );
}

TEST_F( ResourceToolsTest, FowlerNollVoChecksumGeneration )
{
	std::string input = "res:/intromovie.txt";
	std::string output = "";

	EXPECT_TRUE( ResourceTools::GenerateFowlerNollVoChecksum( input, output ) );

	EXPECT_EQ( output, "a9d1721dd5cc6d54" );
}

TEST_F( ResourceToolsTest, DownloadFile )
{
	std::string url = "SomeUrl";
	std::string outputPath = "SomePath";

	EXPECT_TRUE( ResourceTools::DownloadFile( url, outputPath ) );

    // TODO check file exists and is correct using data checksum

}

TEST_F( ResourceToolsTest, GZipCompressData )
{
	std::string inputDataToCompress = "SomeData";

    std::string outputData = "";

	EXPECT_TRUE( ResourceTools::GZipCompressData( inputDataToCompress, outputData ) );

	// TODO check data using real data and data checksum
}

TEST_F( ResourceToolsTest, GZipUncompressData )
{
	std::string inputDataToUncompress = "SomeData";

	std::string outputData = "";

	EXPECT_TRUE( ResourceTools::GZipUncompressData( inputDataToUncompress, outputData ) );

	// TODO check data using real data and data checksum
}

TEST_F( ResourceToolsTest, ResourceChunking )
{
	unsigned long chunkSize = 1000;

	ResourceTools::ChunkStream chunkStream(chunkSize);

    // Add test resource1 data
	std::string resource1Data;

    std::filesystem::path resource1Path = GetTestFileFileAbsolutePath( "Bundle/TestResources/One.png" );

    EXPECT_TRUE(ResourceTools::GetLocalFileData( resource1Path, resource1Data ));

    EXPECT_TRUE(chunkStream << resource1Data);

    std::string resource1Checksum;

    EXPECT_TRUE( ResourceTools::GenerateMd5Checksum( resource1Data, resource1Checksum ));

    // Add test resource2 data
	std::string resource2Data;

	std::filesystem::path resource2Path = GetTestFileFileAbsolutePath( "Bundle/TestResources/Two.png" );

	EXPECT_TRUE( ResourceTools::GetLocalFileData( resource2Path, resource2Data ) );

	EXPECT_TRUE( chunkStream << resource2Data );

    std::string resource2Checksum;

    EXPECT_TRUE( ResourceTools::GenerateMd5Checksum( resource2Data, resource2Checksum ) );

    // Add test resource3 data
	std::string resource3Data;

	std::filesystem::path resource3Path = GetTestFileFileAbsolutePath( "Bundle/TestResources/Three.png" );

	EXPECT_TRUE( ResourceTools::GetLocalFileData( resource3Path, resource3Data ) );

	EXPECT_TRUE( chunkStream << resource3Data );

    std::string resource3Checksum;

	EXPECT_TRUE( ResourceTools::GenerateMd5Checksum( resource3Data, resource3Checksum ) );

    // Get chunks
	int numberOfChunks = 0;

	std::string chunkData;

	ResourceTools::GetChunk chunk;

    chunk.data = &chunkData;

    chunk.clearCache = false;

    while (chunkStream >> chunk)
    {
        // Create Filename
		std::stringstream ss;

        ss << "Chunks/Chunk";

        ss << numberOfChunks;

        ss << ".chunk";

        std::string chunkPath = ss.str();

        // Save chunks
		EXPECT_TRUE( ResourceTools::SaveFile( chunkPath, chunkData ));

        numberOfChunks++;
    }

    // Clear cache for last chunk
	chunk.clearCache = true;

	EXPECT_TRUE( chunkStream >> chunk );

    // Create Filename
	std::stringstream ss;

	ss << "Chunks/Chunk";

	ss << numberOfChunks;

	ss << ".chunk";

	std::string chunkPath = ss.str();

	// Save chunk
	EXPECT_TRUE( ResourceTools::SaveFile( chunkPath, chunkData ) );


	// Reconsitute the files
	ResourceTools::ChunkStream chunkStreamReconstitute( chunkSize );

    for (int i = 0; i < numberOfChunks + 1; i++)
    {
		// Create Filename
		std::stringstream ss;

		ss << "Chunks/Chunk";

		ss << i;

		ss << ".chunk";

		std::string chunkPath = ss.str();

        // Get chunks
		std::string chunkData;

		EXPECT_TRUE( ResourceTools::GetLocalFileData(chunkPath,chunkData));

        EXPECT_TRUE( chunkStreamReconstitute << chunkData );

    }

    // Reconstitute the files and check they match original

    // Resource 1
	std::string reconstitutedResource1Data;

	ResourceTools::GetFile resource1File;

    resource1File.data = &reconstitutedResource1Data;

    resource1File.fileSize = resource1Data.size();

    EXPECT_TRUE( chunkStreamReconstitute >> resource1File );

	std::string reconstitutedResource1Checksum;

	EXPECT_TRUE( ResourceTools::GenerateMd5Checksum( reconstitutedResource1Data, reconstitutedResource1Checksum ) );

    EXPECT_EQ( resource1Checksum, reconstitutedResource1Checksum );

    EXPECT_TRUE( ResourceTools::SaveFile( "Chunks/One.png", reconstitutedResource1Data ) );


    // Resource 2
	std::string reconstitutedResource2Data;

	ResourceTools::GetFile resource2File;

	resource2File.data = &reconstitutedResource2Data;

	resource2File.fileSize = resource2Data.size();

	EXPECT_TRUE( chunkStreamReconstitute >> resource2File );

	std::string reconstitutedResource2Checksum;

	EXPECT_TRUE( ResourceTools::GenerateMd5Checksum( reconstitutedResource2Data, reconstitutedResource2Checksum ) );

	EXPECT_EQ( resource2Checksum, reconstitutedResource2Checksum );

    EXPECT_TRUE( ResourceTools::SaveFile( "Chunks/Two.png", reconstitutedResource2Data ) );


    // Resource 3
	std::string reconstitutedResource3Data;

	ResourceTools::GetFile resource3File;

	resource3File.data = &reconstitutedResource3Data;

	resource3File.fileSize = resource3Data.size();

	EXPECT_TRUE( chunkStreamReconstitute >> resource3File );

	std::string reconstitutedResource3Checksum;

	EXPECT_TRUE( ResourceTools::GenerateMd5Checksum( reconstitutedResource3Data, reconstitutedResource3Checksum ) );

	EXPECT_EQ( resource3Checksum, reconstitutedResource3Checksum );

    EXPECT_TRUE( ResourceTools::SaveFile( "Chunks/Three.png", reconstitutedResource3Data ) );



}