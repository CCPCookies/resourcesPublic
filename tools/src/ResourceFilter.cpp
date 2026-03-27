// Copyright © 2025 CCP ehf.

#include "ResourceFilter.h"
#include <algorithm>

namespace ResourceTools
{

bool ResourceFilter::SetFromFilterFileData( const FilterFile& fileData )
{
	m_paths.clear();

	// Populate prefix paths
	for( auto& prefix : fileData.prefixes )
	{
		for( auto& prefixPath : prefix->paths )
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

        bool containsNonWildcardResPath = false;

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

				filterPath->pathContainsWildcard = false;

				std::replace( filterPath->path.begin(), filterPath->path.end(), '\\', '/' );

				// Remove leading slash
				if( ( filterPath->path.size() > 0 ) && filterPath->path.at( 0 ) == '/' )
				{
					filterPath->path = filterPath->path.substr( 1 );
				}

                filterPath->pathLength = filterPath->path.size();

                bool wildcardReplacedInMatchPattern = false;

				ConvertResPathToPattern( filterPath->path, filterPath->matchPattern, filterPath->pathContainsWildcard );

                if( wildcardReplacedInMatchPattern )
                {
					containsNonWildcardResPath = true;
                }

                filterPath->matchPatternRegex = std::regex( filterPath->matchPattern, std::regex::ECMAScript | std::regex::icase );

				filterPath->prefixId = resPath->prefix->id;

				filterPath->sectionId = filterSection->id;

				filterPath->includeRules = includeRules;

				filterPath->excludeRules = excludeRules;

				// This is important to match files that are matched directly
				// Very specific behaviour is triggered in this case
				filterPath->containsLocalIncludeExcludeRules = ( resPath->includeRules.size() + resPath->excludeRules.size() ) > 0;

				m_paths[filterSection->id].emplace_back( std::move( filterPath ) );
			}
		}

        filterSection->containsNonWildcardResPath = containsNonWildcardResPath;
	}

	return true;
}

void ResourceFilter::ConvertResPathToPattern( std::string resPath, std::string& pattern, bool& pathContainsWildcard ) const
{

	// Replace any "..." with a unique token (RECURSIVE_FOLDER_ELLIPSES_WILDCARD)
	constexpr char RECURSIVE_FOLDER_ELLIPSES_WILDCARD = '\x01';
	size_t pos;
	while( ( pos = resPath.find( "..." ) ) != std::string::npos )
	{
		resPath.replace( pos, 3, std::string( 1, RECURSIVE_FOLDER_ELLIPSES_WILDCARD ) );
	}

	// Escape special characters and deal with wildcards ("*" and "..." i.e. RECURSIVE_FOLDER_ELLIPSES_WILDCARD)
	for( size_t i = 0; i < resPath.size(); ++i )
	{
		if( resPath[i] == '*' )
		{
			pattern += "[^/]*";
			pathContainsWildcard = true;
		}
		else if( resPath[i] == RECURSIVE_FOLDER_ELLIPSES_WILDCARD )
		{
			pattern += ".*";
			pathContainsWildcard = true;
		}
		else if( std::string( ".^$|()[]{}+?\\" ).find( resPath[i] ) != std::string::npos )
		{
			// Regex special characters that need escaping
			pattern += '\\';
			pattern += resPath[i];
		}
		else
		{
			pattern += resPath[i];
		}
	}
}

bool ResourceFilter::CheckPath( const std::string& path, std::string* matchSectionId /*= nullptr*/, std::string* matchPath /*= nullptr*/ ) const
{
	for( auto& sectionFilterPath : m_paths )
	{
		bool includeOrExcludeRulesFailedForSection = false;

		for( auto& filterPath : sectionFilterPath.second )
		{
			// Check for directly specified file
			bool specificFileMatch = false;

			if( !filterPath->pathContainsWildcard ) // If path contains a wildcard it cannot be an exact match
			{
				if( filterPath->pathLength == path.size() ) // If path size doesn't match it doesn't match, early out
				{
					specificFileMatch = filterPath->path == path; // Finally do string comparison
				}
			}

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
					if( matchSectionId )
					{
						( *matchSectionId ) = filterPath->sectionId;
					}
					if( matchPath )
					{
						( *matchPath ) = filterPath->path;
					}

					return true;
				}
			}
			else
			{
				if( !filterPath->pathContainsWildcard )
				{
					// Filter path is not a wildcard
					// And it didn't directly match
					// Therefore the remaining checks can be skipped
					continue;
				}

				// If a previous include or exclude rule was failed
				// Then the rest will also fail
				// The section cannot be completely skipped as
				// There may be specific files specified by full path
				// That don't have to match filter rules.
				if( includeOrExcludeRulesFailedForSection )
				{
					continue;
				}

				// Check exclude rules
				bool excludeRulesPassed = true;

				for( auto& excludeRule : filterPath->excludeRules )
				{
					if( path.find( excludeRule ) != std::string::npos )
					{
						// Exclude rule met
						excludeRulesPassed = false;
						break;
					}
				}

				if( !excludeRulesPassed )
				{
					// exclude rules are present and have not been met
					includeOrExcludeRulesFailedForSection = true;
					continue;
				}

				// Check include rules
				if( !filterPath->includeRules.empty() )
				{
					bool includeRulesPassed = false;

					for( auto& includeRule : filterPath->includeRules )
					{
						if( path.find( includeRule ) != std::string::npos )
						{
							includeRulesPassed = true;
							break;
						}
					}

					if( !includeRulesPassed )
					{
						// Include rules are present and have not been met
						includeOrExcludeRulesFailedForSection = true;
						continue;
					}
				}

				// Perform regex on filter pattern
				try
				{
					if( !std::regex_match( path, filterPath->matchPatternRegex ) )
					{
						continue;
					}
					else
					{
						if( matchSectionId )
						{
							( *matchSectionId ) = filterPath->sectionId;
						}
						if( matchPath )
						{
							( *matchPath ) = filterPath->path;
						}

						return true;
					}
				}
				catch( const std::regex_error& e )
				{
					std::string errorMsg = "Regex Exception during WildcardMatching - regexPattern: " + filterPath->matchPattern + " checkString: " + path + " - error details: " + e.what();
					throw std::runtime_error( errorMsg );
				}
				catch( const std::exception& e )
				{
					std::string errorMsg = "Standard Exception during WildcardMatching - regexPattern: " + filterPath->matchPattern + " checkString: " + path + " - error details: " + e.what();
					throw std::runtime_error( errorMsg );
				}
			}
		}
	}

	return false;
}

const std::vector<std::filesystem::path>& ResourceFilter::GetPrefixPaths() const
{
	return m_prefixPaths;
}

}