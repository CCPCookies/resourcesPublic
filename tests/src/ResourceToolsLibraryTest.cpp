#include <ResourceGroup.h>
#include <BundleResourceGroup.h>
#include <PatchResourceGroup.h>
#include <ResourceTools.h>
#include <filesystem>

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
	const char* FOLDER_NAME = "a9";
	const char* FILE_NAME = "a9d1721dd5cc6d54_e6bbb2df307e5a9527159a4c971034b5";

	const char* testDataPathStr = std::getenv( "TEST_DATA_PATH" );
	ASSERT_TRUE( testDataPathStr );
	std::filesystem::path testDataPath(testDataPathStr);
	ResourceTools::Initialize();

	std::filesystem::path sourcePath = testDataPath / "resourcesLocal" / FOLDER_NAME / FILE_NAME;
	std::string sourcePathString(sourcePath.string());
	std::string url = "file://" + sourcePathString;
	std::filesystem::path outputPath = std::filesystem::temp_directory_path() / FOLDER_NAME / FILE_NAME;
	std::string outputPathString = outputPath.string();

	if( std::filesystem::exists( outputPath ) )
	{
		// Nuke any potentially pre-existing file.
		std::filesystem::remove( outputPath );
	}
	EXPECT_FALSE( std::filesystem::exists( outputPath ) );
	EXPECT_TRUE( ResourceTools::DownloadFile( url, outputPathString ) );
	EXPECT_TRUE( std::filesystem::exists( outputPath ) );

	// Check if download succeeds.
	std::string downloadedData;
	bool success = ResourceTools::GetLocalFileData( outputPathString, downloadedData );
	EXPECT_TRUE( success );

	// Verify downloaded file contents.
	// Note that in this example we are not downloading from an actual web server
	// In production, the web server would return a reply with `Content-Encoding`: `gzip`
	// which would cause curl to automatically unzip the thing for us, but in order to
	// keep the test small and isolated we do not exercise this functionality.
	std::string checksum;
	ResourceTools::GenerateMd5Checksum( downloadedData, checksum );
	EXPECT_STREQ( checksum.c_str(), "6ccf6b7e2e263646f5a78e77b9ba3168" );
	ResourceTools::ShutDown();
}

TEST( ResourceToolsTest, GZipCompressString )
{
	std::string inputDataToCompress = "SomeData";
	std::string outputData = "";
	EXPECT_TRUE( ResourceTools::GZipCompressData( inputDataToCompress, outputData ) );
	std::string expected( "\x1F\x8B\b\0\0\0\0\0\x2\n\v\xCE\xCFMuI,I\x4\0\xB8pH\n\b\0\0\0", 28 );
	EXPECT_EQ( outputData, expected );
}

TEST( ResourceToolsTest, GZipCompressData )
{
	const char* FOLDER_NAME = "a9";
	const char* FILE_NAME = "a9d1721dd5cc6d54_e6bbb2df307e5a9527159a4c971034b5";
	const int GZIP_HEADER_BYTES = 10; // The size of a standard gzip header with no "optional" fields.
	const int FILENAME_BYTES = strlen( FILE_NAME ) + 1; // Number of bytes that the "optional" filename field takes up in the header. 49 characters and one '\0' byte

	const char* testDataPathStr = std::getenv( "TEST_DATA_PATH" );
	ASSERT_TRUE( testDataPathStr );
	std::filesystem::path testDataPath( testDataPathStr );
	std::filesystem::path zippedSourcePath = testDataPath / "resourcesLocal" / FOLDER_NAME / FILE_NAME;
	std::filesystem::path unzippedSourcePath = testDataPath / "resourcesOnBranch" / "introMovie.txt";

	std::string zippedFileData;
	ASSERT_TRUE( ResourceTools::GetLocalFileData( zippedSourcePath, zippedFileData ) );

	std::string unzippedFileData;
	ASSERT_TRUE( ResourceTools::GetLocalFileData( unzippedSourcePath, unzippedFileData ) );

	std::string compressed;
	EXPECT_TRUE( ResourceTools::GZipCompressData( unzippedFileData, compressed ) );

	// Check that the compressed file matches the data in the file we have on disk
	// EXCEPT for the header.
	// https://docs.fileformat.com/compression/gz/
	std::string compressedNoHeader = compressed.substr( GZIP_HEADER_BYTES );
	std::string zippedFileDataNoHeader = zippedFileData.substr( GZIP_HEADER_BYTES + FILENAME_BYTES );
	EXPECT_EQ( compressedNoHeader, zippedFileDataNoHeader );
}

TEST( ResourceToolsTest, GZipUncompressString )
{
	std::string inputDataToUncompress( "\x1F\x8B\b\0\0\0\0\0\x2\n\v\xCE\xCFMuI,I\x4\0\xB8pH\n\b\0\0\0", 28 );
	std::string outputData = "";
	EXPECT_TRUE( ResourceTools::GZipUncompressData( inputDataToUncompress, outputData ) );
	EXPECT_EQ( outputData, "SomeData" );
}

TEST( ResourceToolsTest, GZipUncompressData )
{
	const char* FOLDER_NAME = "a9";
	const char* FILE_NAME = "a9d1721dd5cc6d54_e6bbb2df307e5a9527159a4c971034b5";
	const char* testDataPathStr = std::getenv( "TEST_DATA_PATH" );
	ASSERT_TRUE( testDataPathStr );

	std::filesystem::path testDataPath( testDataPathStr );
	std::filesystem::path zippedSourcePath = testDataPath / "resourcesLocal" / FOLDER_NAME / FILE_NAME;
	std::filesystem::path unzippedSourcePath = testDataPath / "resourcesOnBranch" / "introMovie.txt";

	// Load gzipped file.
	std::string zippedFileData;
	ASSERT_TRUE( ResourceTools::GetLocalFileData( zippedSourcePath, zippedFileData ) );

	// Decompress the file.
	std::string decompressed;
	EXPECT_TRUE( ResourceTools::GZipUncompressData( zippedFileData, decompressed ) );

	// Make sure decompressed file matches expected contents.
	std::string checksum;
	ResourceTools::GenerateMd5Checksum( decompressed, checksum );
	EXPECT_EQ( checksum, "e6bbb2df307e5a9527159a4c971034b5" );
}
