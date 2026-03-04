// Copyright © 2025 CCP ehf.

#include "ResourceFilter.h"

#include <regex>
#include <algorithm>

namespace ResourceTools
{

ResourceFilter::ResourceFilter()
{
}

ResourceFilter::~ResourceFilter()
{
}

bool ResourceFilter::SetFromFilterFileData( const FilterFile& fileData )
{
	m_paths.clear();

	// Populate prefix paths
	for( auto& prefix : fileData.prefixes )
	{
		for( auto& prefixPath : prefix.second->paths )
		{
			m_prefixPaths.push_back( prefixPath );
		}
	}

	// Populate search paths from filter data
	for( auto& filterSection : fileData.filterSections )
	{
		std::set<std::string> includeRules = fileData.includeRules;
		std::set<std::string> excludeRules = fileData.excludeRules;

		// Add section rules
		includeRules.insert( filterSection->includeRules.begin(), filterSection->includeRules.end() );
		excludeRules.insert( filterSection->excludeRules.begin(), filterSection->excludeRules.end() );

		for( auto& resPath : filterSection->respaths )
		{
			for( auto& prefixPath : resPath->prefix->paths )
			{
				// Add include rules to section rules
				includeRules.insert( resPath->includeRules.begin(), resPath->includeRules.end() );

				excludeRules.insert( resPath->excludeRules.begin(), resPath->excludeRules.end() );

				// Create a filter path
				std::unique_ptr<FilterPath> filterPath = std::make_unique<FilterPath>();

				// Normalise path and convert to pattern
				std::string prefixPathStr = prefixPath.string();

				// Remove current directory dot if supplied
				if( prefixPathStr == "." )
				{
					prefixPathStr = "";
				}

				// ResolvePath
				filterPath->path = prefixPathStr + resPath->path.string();

				std::replace( filterPath->path.begin(), filterPath->path.end(), '\\', '/' );

				// Remove leading slash
				if( ( filterPath->path.size() > 0 ) && filterPath->path.at( 0 ) == '/' )
				{
					filterPath->path = filterPath->path.substr( 1 );
				}

				ConvertResPathToPattern( filterPath->path, filterPath->matchPattern );

				filterPath->prefixId = resPath->prefix->id;

				filterPath->sectionId = filterSection->id;

				filterPath->includeRules = includeRules;

				filterPath->excludeRules = excludeRules;

				// This is important to match files that are matched directly
				// Very specific behaviour is triggered in this case
				filterPath->containsLocalIncludeExcludeRules = ( resPath->includeRules.size() + resPath->excludeRules.size() ) > 0;

				m_paths.emplace_back( std::move( filterPath ) );
			}
		}
	}

	return true;
}

void ResourceFilter::ConvertResPathToPattern( const std::string& resPath, std::string& pattern ) const
{
	std::string resPathString = resPath;

	// Replace any "..." with a unique token (RECURSIVE_FOLDER_ELLIPSES_WILDCARD)
	constexpr char RECURSIVE_FOLDER_ELLIPSES_WILDCARD = '\x01';
	size_t pos;
	while( ( pos = resPathString.find( "..." ) ) != std::string::npos )
	{
		resPathString.replace( pos, 3, std::string( 1, RECURSIVE_FOLDER_ELLIPSES_WILDCARD ) );
	}

	// Escape special characters and deal with wildcards ("*" and "..." i.e. RECURSIVE_FOLDER_ELLIPSES_WILDCARD)
	for( size_t i = 0; i < resPathString.size(); ++i )
	{
		if( resPathString[i] == '*' )
		{
			pattern += "[^/]*";
		}
		else if( resPathString[i] == RECURSIVE_FOLDER_ELLIPSES_WILDCARD )
		{
			pattern += ".*";
		}
		else if( std::string( ".^$|()[]{}+?\\" ).find( resPathString[i] ) != std::string::npos )
		{
			// Regex special characters that need escaping
			pattern += '\\';
			pattern += resPathString[i];
		}
		else
		{
			pattern += resPathString[i];
		}
	}
}

bool ResourceFilter::CheckPath( const std::filesystem::path& path ) const
{
	std::string sectionId;
	std::string matchPath;
	return CheckPath( path, sectionId, matchPath );
}

bool ResourceFilter::CheckPath( const std::filesystem::path& path, std::string& matchSectionId, std::string& matchPath ) const
{
	for( auto& filterPath : m_paths )
	{
		std::string resolvedPathStr = path.string();

		std::replace( resolvedPathStr.begin(), resolvedPathStr.end(), '\\', '/' );

		// Check for directly specified file
		bool specificFileMatch = filterPath->path == resolvedPathStr;

		if( !specificFileMatch )
		{
			// Attempt to match using pattern
			try
			{
				std::regex re( filterPath->matchPattern, std::regex::ECMAScript | std::regex::icase );
				if( !std::regex_match( resolvedPathStr, re ) )
				{
					continue;
				}
			}
			catch( const std::regex_error& e )
			{
				std::string errorMsg = "Regex Exception during WildcardMatching - regexPattern: " + filterPath->matchPattern + " checkString: " + resolvedPathStr + " - error details: " + e.what();
				throw std::runtime_error( errorMsg );
			}
			catch( const std::exception& e )
			{
				std::string errorMsg = "Standard Exception during WildcardMatching - regexPattern: " + filterPath->matchPattern + " checkString: " + resolvedPathStr + " - error details: " + e.what();
				throw std::runtime_error( errorMsg );
			}
		}

		// Apply include/exclude rules
		if( specificFileMatch )
		{
			if( filterPath->containsLocalIncludeExcludeRules )
			{
				// If a local include or exclude is defined here then always fail
				// This is to match behaviour in current system, likely a bug
				continue;
			}
			else
			{
				// Ignore all other rules and match this file
				matchSectionId = filterPath->sectionId;
				matchPath = filterPath->path;
				return true;
			}
		}

		// Includes
		if( !filterPath->includeRules.empty() )
		{
			bool includeRulesPassed = false;

			for( auto& includeRule : filterPath->includeRules )
			{
				if( resolvedPathStr.find( includeRule ) != std::string::npos )
				{
					includeRulesPassed = true;
					break;
				}
			}

			if( !includeRulesPassed )
			{
				// Include rules are present and have not been met
				continue;
			}
		}

		// Excludes
		bool excludeRulesPassed = true;

		for( auto& excludeRule : filterPath->excludeRules )
		{
			if( resolvedPathStr.find( excludeRule ) != std::string::npos )
			{
				// Exclude rule met
				excludeRulesPassed = false;
				break;
			}
		}

		if( !excludeRulesPassed )
		{
			// Include rules are present and have not been met
			continue;
		}

		// Pattern matched and include/exclude rules met
		matchSectionId = filterPath->sectionId;
		matchPath = filterPath->path;
		return true;
	}

	return false;
}

const std::vector<std::filesystem::path>& ResourceFilter::GetPrefixPaths() const
{
	return m_prefixPaths;
}

}