////
//  Commonly used in exes/Engine, dlls/Game.
////

#pragma once
#include <memory>
#include <unordered_map>
#include "Hash.h"
#include "IConsole.h"
#include "ISound.h"

class IServiceLocator
{
public:
	virtual ~IServiceLocator( ) = default;

	virtual IConsole& console( ) = 0;
	virtual auto vault( ) -> std::unordered_map<HashedKey, Dword>& = 0;
	virtual auto sound( ) -> ISound& = 0;
protected:
	IServiceLocator( ) = default;
};