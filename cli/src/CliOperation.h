/* 
	*************************************************************************

	CliOperation.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef CliOperation_H
#define CliOperation_H

#include <string>

#include <Enums.h>

namespace argparse
{
class ArgumentParser;
}

class CliOperation
{
public:

	CliOperation( const std::string& name, const std::string& description );

	~CliOperation();

	argparse::ArgumentParser* GetParser() const;

	void PrintError() const;

	virtual bool Execute() const = 0;

protected:

    void PrintCarbonResourcesError( CarbonResources::Result result ) const;

    bool AddRequiredPositionalArgument( const std::string& argumentId, const std::string& helpString );

	bool AddArgument( const std::string& argumentId, const std::string& shortArgumentId, const std::string& helpString, bool required = false, std::string defualtValue = "" );

    static void StatusUpdate( int progress, const std::string& info );

	argparse::ArgumentParser* m_argumentParser;

private:
	std::string m_name;

	std::string m_description;
};

#endif // CliOperation_H