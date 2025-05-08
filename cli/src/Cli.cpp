#include "Cli.h"

#include <argparse/argparse.hpp>

Cli::Cli( const std::string& name, const std::string& version )
{
	m_argumentParser = new argparse::ArgumentParser( name );
}

Cli::~Cli()
{
	if( m_argumentParser )
	{
		delete m_argumentParser;
	}
}

bool Cli::AddOperation( const CliOperation* operation )
{
	if( !m_argumentParser )
	{
		return false;
	}

	m_argumentParser->add_subparser( *operation->GetParser() );

	m_operations.push_back( operation );
}

void Cli::PrintError()
{
	std::cout << *m_argumentParser;
}

bool Cli::ProcessCommandLine( int argc, char** argv )
{
	try
	{
		m_argumentParser->parse_args( argc, argv );
	}
	catch( const std::runtime_error& e )
	{
		std::cerr << e.what() << std::endl;

		for( const CliOperation* operation : m_operations )
		{
			if( m_argumentParser->is_subcommand_used( *operation->GetParser() ) )
			{
				operation->PrintError();
			}
		}

		return false;
	}

	return true;
}

void Cli::PrintCliHeader()
{
	std::cout << "====================" << std::endl;
	std::cout << "carbon-resources-cli" << std::endl;

    std::stringstream ss;

	ss << CarbonResources::S_LIBRARY_VERSION.major << "." << CarbonResources::S_LIBRARY_VERSION.minor << "." << CarbonResources::S_LIBRARY_VERSION.patch;

	std::cout << "carbon-resources version: " << ss.str() << std::endl;

    std::cout << "====================\n" << std::endl;

}

bool Cli::Execute()
{
	for( const CliOperation* operation : m_operations )
	{
		if( m_argumentParser->is_subcommand_used( *operation->GetParser() ) )
		{
			PrintCliHeader();

			return operation->Execute();
		}
	}

	PrintError();

	return false;
}