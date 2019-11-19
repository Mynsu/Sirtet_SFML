////
// Game API
////

#pragma once
#include <memory>
#include <unordered_map>
#include <Lib/Hash.h>
#include "IConsole.h"

class IServiceLocator
{
public:
	virtual ~IServiceLocator( ) = default;

	virtual IConsole* console( ) = 0;
	virtual auto vault( ) -> std::unordered_map< HashedKey, Dword >& = 0;
protected:
	IServiceLocator( ) = default;
};