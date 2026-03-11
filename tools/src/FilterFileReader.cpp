// Copyright © 2025 CCP ehf.

#include "FilterFileReader.h"

#include <INIReader.h>
#include <cctype>
#include <sstream>
#include <algorithm>

namespace ResourceTools
{

FilterFileReader::FilterFileReader()
{
}

FilterFileReader::~FilterFileReader()
{
}

void FilterFileReader::LoadFromIniFileData( const char* data, size_t dataSize, FilterFile& fileData, bool ignoreCase /*= false*/ )
{

	INIReader reader( data, dataSize );

	if( reader.ParseError() != 0 )
	{
		throw std::runtime_error( "Failed to parse INI file - " + reader.ParseErrorMessage() );
	}

	if( !reader.HasSection( "DEFAULT" ) )
	{
		throw std::invalid_argument( "Required [DEFAULT] section not present in INI file. " );
	}

	// Get prefix information
	std::string prefixMappingsStr = reader.Get( "DEFAULT", "prefixmap", "" );

	ParsePrefixMappings( prefixMappingsStr, fileData.prefixes );

	std::string globalFiltersStr = reader.Get( "DEFAULT", "filter", "" );

	ParseIncludeExcludeRules( globalFiltersStr, fileData.includeRules, fileData.excludeRules, ignoreCase );

	// Get section information
	for( const auto& sectionName : reader.Sections() )
	{
		if( sectionName == "default" || sectionName == "DEFAULT" )
		{
			continue; // Already loaded, skip it
		}

		std::unique_ptr filterSection = std::make_unique<FilterSection>();

		filterSection->id = sectionName;

		// Filter is optional
		std::string filter = reader.Get( sectionName, "filter", "" );

		ParseIncludeExcludeRules( filter, filterSection->includeRules, filterSection->excludeRules, ignoreCase );

		// Respaths is required
		std::string respathsStr = reader.Get( sectionName, "respaths", "" );

		ParseSectionResPathEntry( respathsStr, filterSection->respaths, fileData.prefixes, ignoreCase );

		fileData.filterSections.push_back( std::move( filterSection ) );
	}
}

void FilterFileReader::ParsePrefixMappings( const std::string& prefixStr, std::vector<std::shared_ptr<Prefix>>& prefixes )
{

	std::size_t pos = 0;
	while( pos < prefixStr.size() )
	{
		// Find the prefix (or error out if missing a colon ":")
		std::size_t colon = prefixStr.find( ':', pos );
		if( colon == std::string::npos )
		{
			throw std::invalid_argument( "Invalid prefixmap format: missing ':'" );
		}

		std::string prefix = prefixStr.substr( pos, colon - pos );
		if( prefix.empty() )
		{
			throw std::invalid_argument( "Invalid prefixmap format: empty prefix" );
		}

		// Move position past the colon
		pos = colon + 1;

		// Find end of paths (next whitespace or end of string)
		std::size_t nextSpace = prefixStr.find_first_of( " \t\r\n", pos );
		std::string rawPaths = ( nextSpace == std::string::npos ) ?
			prefixStr.substr( pos ) :
			prefixStr.substr( pos, nextSpace - pos );

		if( rawPaths.empty() )
		{
			throw std::invalid_argument( "Invalid prefixmap format: No paths defined for prefix: " + prefix );
		}

        bool prefixFound = false;

		std::shared_ptr<Prefix> currentPrefix;

		for( auto iter = prefixes.begin(); iter != prefixes.end(); iter++ )
		{
			if( ( *iter )->id == prefix )
			{
				prefixFound = true;
				currentPrefix = *iter;
				break;
			}
		}

		if( !prefixFound )
		{
			currentPrefix = std::make_shared<Prefix>();

			currentPrefix->id = prefix;

			prefixes.push_back( currentPrefix );
		}

		ParsePrefixPaths( rawPaths, currentPrefix->paths );

		// Go to the next token in the rawPrefixMap (or break if at end)
		if( nextSpace == std::string::npos )
		{
			break;
		}
		pos = nextSpace + 1;

		// There was a whitespace, skip any additional spaces as well
		while( pos < prefixStr.size() && std::isspace( static_cast<unsigned char>( prefixStr[pos] ) ) )
		{
			++pos;
		}
	}
}

void FilterFileReader::ParsePrefixPaths( const std::string& prefixPathsStr, std::vector<std::filesystem::path>& paths )
{
	std::size_t pos = 0;
	// Loop through rawPaths and split by semicolons to extract individual paths (in case there are many)
	while( pos < prefixPathsStr.size() )
	{
		// Split the string up by semicolons (in case of multiple paths)
		std::size_t semicolon = prefixPathsStr.find( ';', pos );
		std::string path = ( semicolon == std::string::npos ) ?
			prefixPathsStr.substr( pos ) :
			prefixPathsStr.substr( pos, semicolon - pos );

		if( !path.empty() )
		{
			paths.push_back( path );
		}

		if( semicolon == std::string::npos )
		{
			break;
		}
		pos = semicolon + 1;
	}
}

void FilterFileReader::ParseIncludeExcludeRules( const std::string& rulesStr, std::set<std::string>& includeRules, std::set<std::string>& excludeRules, bool ignoreCase )
{

	size_t pos = 0;
	while( pos < rulesStr.size() )
	{
		// Skip whitespaces
		while( pos < rulesStr.size() && std::isspace( static_cast<unsigned char>( rulesStr[pos] ) ) )
		{
			++pos;
		}
		if( pos >= rulesStr.size() )
		{
			break;
		}

		// Check for exclude filter marker '!'
		bool isExclude = false;
		if( rulesStr[pos] == '!' )
		{
			// We have an exclude filter, advance the position by one and skip whitespace(s)
			isExclude = true;
			++pos;
			while( pos < rulesStr.size() && std::isspace( static_cast<unsigned char>( rulesStr[pos] ) ) )
			{
				++pos;
			}
			if( pos >= rulesStr.size() )
			{
				throw std::invalid_argument( "Invalid filter format: exclude filter marker found without a [ token ] section" );
			}
		}

		if( pos >= rulesStr.size() || rulesStr[pos] != '[' )
		{
			throw std::invalid_argument( "Invalid filter format: missing '['" );
		}
		++pos; // skip '['

		size_t endBracket = rulesStr.find( ']', pos );
		size_t nextStartBracket = rulesStr.find( '[', pos );
		if( nextStartBracket != std::string::npos && nextStartBracket < endBracket )
		{
			throw std::invalid_argument( "Invalid filter format: matching end bracket ']' not present before the next start bracket '['" );
		}

		if( endBracket == std::string::npos )
		{
			throw std::invalid_argument( "Invalid filter format: missing ']'" );
		}

		std::string entries = rulesStr.substr( pos, endBracket - pos );
		std::istringstream iss( entries );
		std::string token;
		while( iss >> token )
		{
			// Trim whitespace from token
			size_t start = token.find_first_not_of( " \t\r\n" );
			size_t end = token.find_last_not_of( " \t\r\n" );
			if( start == std::string::npos || end == std::string::npos )
			{
				continue;
			}
			token = token.substr( start, end - start + 1 );

			if( token.empty() )
			{
				continue;
			}

            if (ignoreCase)
            {
				std::transform( token.begin(), token.end(), token.begin(), []( unsigned char c ) { return std::tolower( c ); } );
            }

			if( isExclude )
			{
				excludeRules.insert( token );
			}
			else
			{
				includeRules.insert( token );
			}
		}
		pos = endBracket + 1;
	}
}

void FilterFileReader::ParseSectionResPathEntry( const std::string& filterStr, std::vector<std::unique_ptr<ResPath>>& resPaths, std::vector<std::shared_ptr<Prefix>>& prefixes, bool ignoreCase )
{

	// Split rawPathFileAttrib into lines (in case of multiline attribute)
	std::istringstream stream( filterStr );
	std::string line;

	while( std::getline( stream, line ) )
	{
		// Trim whitespace from both ends
		size_t first = line.find_first_not_of( " \t\r" );
		if( first == std::string::npos )
		{
			continue; // skip if empty line
		}

		size_t last = line.find_last_not_of( " \t\r" );
		std::string rawPathLine = line.substr( first, last - first + 1 );

		// Skip commented out lines (in case there is "inline" comment within the .ini file attribute value)
		if( rawPathLine.empty() || rawPathLine[0] == '#' || rawPathLine[0] == ';' )
		{
			continue;
		}

		std::istringstream iss( rawPathLine );
		std::string rawPrefixPathToken;
		iss >> rawPrefixPathToken;

		size_t colon = rawPrefixPathToken.find( ':' );
		if( colon == std::string::npos )
		{
			throw std::invalid_argument( std::string( "Missing prefix in path for: " ) + rawPathLine );
		}
		std::string prefixPart = rawPrefixPathToken.substr( 0, colon );
		std::string pathPart = rawPrefixPathToken.substr( colon + 1 );

		if( pathPart.find( "../" ) != std::string::npos )
		{
			// Escaping is not supported in respaths
			throw std::invalid_argument( "Escaping paths not supported for respaths: " + rawPathLine );
		}

		std::unique_ptr resPath = std::make_unique<ResPath>();

        if (ignoreCase)
        {
			std::transform( pathPart.begin(), pathPart.end(), pathPart.begin(), []( unsigned char c ) { return std::tolower( c ); } );
        }

		resPath->path = pathPart;

        auto prefixIter = prefixes.begin();

		for( prefixIter; prefixIter != prefixes.end(); prefixIter++ )
		{
			if( ( *prefixIter )->id == prefixPart )
			{
				break;
			}
		}

		if( prefixIter == prefixes.end() )
		{
			throw std::invalid_argument( "Respath referencing unknown prefix: " + rawPathLine );
		}

		resPath->prefix = *prefixIter;

		if( !iss.eof() )
		{
			// Read optional filters
			std::string rawOptionalFilterPart;
			std::getline( iss, rawOptionalFilterPart );

			ParseIncludeExcludeRules( rawOptionalFilterPart, resPath->includeRules, resPath->excludeRules, ignoreCase );
		}

		resPaths.push_back( std::move( resPath ) );
	}
}

}