/* 
	*************************************************************************

	Resource.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/

#pragma once
#ifndef Resource_H
#define Resource_H

#include "Exports.h"
#include "Enums.h"
#include <memory>
#include <string>
#include <optional>
#include <vector>

namespace YAML
{
    class Emitter;
    class Node;
}

namespace CarbonResources
{
    
    struct API ResourceParams
    {
		std::string relativePath = "";

		std::string location = "";

		std::string checksum = "";

		unsigned long compressedSize = 0;

		unsigned long uncompressedSize = 0;

		unsigned long something = 0;
    };

    struct API ResourceSourceSettings
    {
		std::string developmentLocalBasePath = "";

		std::string productionLocalBasePath = "";

		std::string productionRemoteBaseUrl = "";
    };

    struct API ResourceDestinationSettings
    {
		std::string developmentLocalBasePath = "";  // TODO needs to be supported and also needs a rename, it's not just developmentLocal now

		std::string productionLocalBasePath = "";
    };

    struct API ResourceGetDataParams
	{
		ResourceSourceSettings resourceSourceSettings;

        std::string data;

	};
  
    class ResourceImpl;
	class PatchResourceGroupImpl;

    //TODO remove Resource from public API and remove pimpl style
    class API Resource
    {
    protected:
        Resource( ResourceImpl* impl );

        ResourceImpl* m_impl;

    public:
		Resource( const ResourceParams& params );   // TODO this should not be possible publicly

	    virtual ~Resource();

        bool operator==( const Resource* other ) const  // TODO remove from public API
		{
			return (GetRelativePath() == other->GetRelativePath()) && (GetChecksum() == other->GetChecksum());
		}

        std::string GetRelativePath() const; // TODO should return a base type, DocumentParameter is internal

        std::string GetLocation() const;

        std::string GetChecksum() const;

        unsigned long GetUncompressedSize() const;

        unsigned long GetCompressedSize() const;

        unsigned long GetSomething() const;

		Result GetData( ResourceGetDataParams& params ) const;  //TODO remove from public API

        friend class ResourceGroupImpl;
		friend class PatchResourceGroupImpl;
    };

}

#endif // Resource_H