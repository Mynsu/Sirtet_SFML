////
//  Shared by exes/Engine, dlls/Game.
////

#pragma once
#include <memory>
#include <unordered_map>
#include "../Lib/Hash.h"
#include "IConsole.h"
#include "ISound.h"

class IServiceLocator
{
public:
	virtual ~IServiceLocator( ) = default;

	virtual IConsole& console( ) = 0;
	virtual auto vault( ) -> std::unordered_map<HashedKey, Dword>& = 0;
	virtual void provideSound( std::unique_ptr<ISound>& soundService ) = 0;
	virtual auto sound( ) -> ISound& = 0;
protected:
	IServiceLocator( ) = default;
};