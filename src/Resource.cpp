#include "Resource.h"

#include "ResourceImpl.h"

namespace CarbonResources
{

    Resource::Resource( const ResourceParams& params ) :
	    m_impl( new ResourceImpl(params) )
    {
    }

    Resource::Resource( ResourceImpl* impl ) :
		m_impl( impl )
	{
	}

    Resource::~Resource()
    {
		delete m_impl;
    }

    /// @brief Returns relative path of a resource
    /// @param data data which the checksum will be based on
    /// @param data_size size of data passed in
    /// @param checksum will contain the resulting checksum on success
    /// @return true on success, false on failure
	std::string Resource::GetRelativePath() const
    {
		return m_impl->GetRelativePath().GetValue().ToString();
    }

    /// @brief Returns cdn location of a resource
    /// @param data data which the checksum will be based on
    /// @param data_size size of data passed in
    /// @param checksum will contain the resulting checksum on success
    /// @return true on success, false on failure
    std::string Resource::GetLocation() const
    {
		return m_impl->GetLocation().GetValue();
    }

    /// @brief Returns data checksum of a resource
    /// @param data data which the checksum will be based on
    /// @param data_size size of data passed in
    /// @param checksum will contain the resulting checksum on success
    /// @return true on success, false on failure
    std::string Resource::GetChecksum() const
    {
		return m_impl->GetChecksum().GetValue();
    }

    /// @brief Returns uncompressed size of resource
    /// @param data data which the checksum will be based on
    /// @param data_size size of data passed in
    /// @param checksum will contain the resulting checksum on success
    /// @return true on success, false on failure
    unsigned long Resource::GetUncompressedSize() const
    {
		return m_impl->GetUncompressedSize().GetValue();
    }

    /// @brief Returns compressed size of resource
    /// @param data data which the checksum will be based on
    /// @param data_size size of data passed in
    /// @param checksum will contain the resulting checksum on success
    /// @return true on success, false on failure
    unsigned long Resource::GetCompressedSize() const
    {
		return m_impl->GetCompressedSize().GetValue();
    }


    // TODO remove this something tag, it is just a thought exercise
    unsigned long Resource::GetSomething() const
    {
		return m_impl->GetSomething().GetValue();
    }

    
    Result Resource::GetData( ResourceGetDataParams& params ) const
    {
		return m_impl->GetData( params );
    }
    

}