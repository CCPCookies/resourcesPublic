/* 
	*************************************************************************

	BundleStreamOut.h

	Author:    James Hawk
	Created:   March. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/

#pragma once
#ifndef BundleStreamOut_H
#define BundleStreamOut_H

#include <string>

namespace ResourceTools
{

    struct GetChunk
	{
		std::string* data = nullptr;

        bool clearCache = false;
	};

    class BundleStreamOut
    {
	public:
		BundleStreamOut( uintmax_t chunkSize );

		~BundleStreamOut();

        bool operator<<( const std::string& data ); 

        // Outputs chunks
		bool operator>>( GetChunk& data );

    	uintmax_t GetChunkSize() const;

    	bool ReadBytes( size_t n, std::string& data );

    private:

		uintmax_t m_chunkSize;

        std::string m_cache;

    };
 
}

#endif // BundleStreamOut_H