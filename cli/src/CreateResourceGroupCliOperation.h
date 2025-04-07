/* 
	*************************************************************************

	CreateResourceGroupCliOperation.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef CreateResourceGroupCliOperation_H
#define CreateResourceGroupCliOperation_H

#include <filesystem>

#include "CliOperation.h"

class CreateResourceGroupCliOperation : public CliOperation
{
public:
	CreateResourceGroupCliOperation();

	virtual bool Execute() const override;

private:

	bool CreateResourceGroup( const std::filesystem::path& inputDirectory, const std::filesystem::path& resourceGroupOutputDirectory ) const;

private:

	std::string m_createResourceGroupPathArgumentId;

	std::string m_createResourceGroupOutputFilenameArgumentId;
};

#endif // CreateResourceGroupCliOperation_H