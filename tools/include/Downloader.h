// Copyright © 2025 CCP ehf.

#pragma once

#include <filesystem>
#include <string>

#include <curl/curl.h>

namespace ResourceTools
{

enum class Response
{
    NONE,
	SUCCESS,
	FILE_NOT_FOUND,
	DOWNLOAD_ERROR,
};

// Utility class for downloading files.
// Reuse is encouraged for multiple downloads, but do not share across threads.
class Downloader
{
public:
	Downloader();

	~Downloader();

	bool DownloadFile( const std::string& url, const std::filesystem::path& outputPath, const std::chrono::seconds& retrySeconds );

    Response GetHeader( const std::string& url, std::string& response );

	static bool GetAttributeValueFromHeader( const std::string& header, const std::string& attributeName, std::string& value );

private:
	CURL* m_curlHandle{ nullptr };
};
}