
#include "CompressedFileDataStreamOut.h"

namespace ResourceTools
{

  CompressedFileDataStreamOut::CompressedFileDataStreamOut( ):
	m_compressionStream(nullptr)
  {
  }

  CompressedFileDataStreamOut::~CompressedFileDataStreamOut()
  {
      if (m_compressionStream)
      {
		  delete m_compressionStream;
      }
  }

  bool CompressedFileDataStreamOut::StartWrite( std::filesystem::path filepath )
  {
	  m_compressionStream = new GzipCompressionStream( &m_compressionBuffer );
      
      if (!m_compressionStream->Start())
      {
		  return false;
      }

      return FileDataStreamOut::StartWrite( filepath );
  }

  bool CompressedFileDataStreamOut::Finish()
  {
      if (!m_compressionStream)
      {
		  return false;
      }

      if (!m_compressionStream->Finish())
      {
		  return false;
      }

      if( m_compressionBuffer.size() > 0 )
	  {
          if (!FileDataStreamOut::operator<<(m_compressionBuffer))
          {
			  return false;
          }
	  }

      delete m_compressionStream;

      m_compressionStream = nullptr;

      m_compressionBuffer.clear();

      return FileDataStreamOut::Finish();
  }

  bool CompressedFileDataStreamOut::operator << ( const std::string& data )
  {
	  if( !m_compressionStream )
	  {
		  return false;
	  }

      if (!m_compressionStream->operator<<(&data))
      {
		  return false;
      }

      if (m_compressionBuffer.size() > 0)
      {
          if (!FileDataStreamOut::operator<<(m_compressionBuffer))
          {
			  return false;
          }

          m_compressionBuffer.clear();
      }

      return true;
  }


}