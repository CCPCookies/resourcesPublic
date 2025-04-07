#include "CliOperation.h"

#include <argparse/argparse.hpp>

CliOperation::CliOperation( const std::string& name, const std::string& description ) :
	m_name( name ),
	m_description( description ),
	m_argumentParser( nullptr )
{
	m_argumentParser = new argparse::ArgumentParser( name );
}

CliOperation ::~CliOperation()
{
	if( m_argumentParser )
	{
		delete m_argumentParser;
	}
}

bool CliOperation::AddRequiredPositionalArgument( const std::string& argumentId, const std::string& helpString )
{
	if( !m_argumentParser )
	{
		return false;
	}

	m_argumentParser->add_argument( argumentId )
		.help( helpString );
}

bool CliOperation::AddArgument( const std::string& argumentId, const std::string& shortArgumentId, const std::string& helpString, bool required /* = false*/, std::string defualtValue /*= ""*/ )
{
	if( !m_argumentParser )
	{
		return false;
	}

	argparse::Argument argument = m_argumentParser->add_argument( shortArgumentId, argumentId )
									  .help( helpString );

	if( required )
	{
		argument.required();
	}

	if( defualtValue != "" )
	{
		argument.default_value( defualtValue );
	}
}

argparse::ArgumentParser* CliOperation::GetParser() const
{
	return m_argumentParser;
}

void CliOperation::PrintError() const
{
	std::cout << *m_argumentParser;
}

void CliOperation::PrintCarbonResourcesError( CarbonResources::Result result ) const
{
	std::string errorMessage;

	bool ret = CarbonResources::resultToString( result, errorMessage );

	std::cout << errorMessage << "\n"
			  << std::endl;
}

// TODO this interface needs work
void CliOperation::StatusUpdate( int progress, const std::string& info )
{
	if( info == "Percentage Update" )
	{
		std::cout << "[";

		for( int i = 0; i < 100; i++ )
		{
			if( i < progress )
			{
				std::cout << "=";
			}
			else if( i == progress )
			{
				std::cout << ">";
			}
			else
			{
				std::cout << " ";
			}
		}

		std::cout << "]" << progress << " %\r";

		std::cout.flush();
	}
	else
	{
		std::cout << info << std::endl;
	}
}