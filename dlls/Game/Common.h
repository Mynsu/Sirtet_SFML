//TODO: precompile할까?

#pragma once
#pragma hdrstop

#ifdef GAME_EXPORTS
#define GAME_API __declspec( dllexport )
#else
#define GAME_API __declspec( dllimport )
#endif

// This is not for the purpose of using static member functions,
// just sharing the header files.
#include "../exes/Engine/ServiceLocator.h"
namespace global
{
	// 궁금: 이거 안 되는데, 왜?
	///GAME_API auto ( *Console )( ) -> std::unique_ptr< IConsole >&;
	extern GAME_API std::unique_ptr< IConsole >& ( *Console )( );
	extern GAME_API std::unordered_map< hashValue_t, dword >& ( *VariableTable )( );
}

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

#ifdef _DEBUG
#define ASSERT_FALSE( x ) if ( true == x ) __debugbreak( )
#define ASSERT_TRUE( x ) if ( false == x ) __debugbreak( )
#else
#define ASSERT_FALSE( x ) x
#define ASSERT_TRUE( x ) x
#endif
