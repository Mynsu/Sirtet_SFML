#pragma once
#include <variant>
#include <string>
#include <unordered_map>
#include <lua.hpp>
#ifdef GAME_EXPORTS
#include "../dlls/Game/ServiceLocatorMirror.h"
#else
#include "../exes/Engine/ServiceLocator.h"
#endif

namespace util::script
{
	// Load data from a .lua script file.
	// Argument's type should be const char[] or const char*.
	// Value type is std::variant<bool, int, float, std::string>, which you can customize.
	// This throws std::runtime_error when failing to open the script file.
	template < typename Value=std::variant<bool,int,float,std::string>, typename Cstr, typename... Cstrs,
		std::enable_if_t<std::is_same_v<std::decay_t<Cstr>,const char*>>* = nullptr >
		static const std::unordered_map< Cstr, Value > LoadFromScript( Cstr scriptPathNName, Cstrs... variables )//TODO: 패키지에 모은 후 암호화
	{
		lua_State* lua = luaL_newstate( );
		// Exception handling
		// Make sure that this function returns true on failure.
		if ( true == luaL_dofile(lua, scriptPathNName) )
		{
			lua_close( lua );
			
#ifdef GAME_EXPORTS
			(*glpService).console()->printFailure( FailureLevel::FATAL, std::string("File Not Found: ")+scriptPathNName );
#else
			gService.console( )->printFailure( FailureLevel::FATAL, std::string("File Not Found: ")+scriptPathNName );
#endif

#ifdef _DEBUG
			__debugbreak( );
#else
			throw std::runtime_error( "" );
#endif
		}
		luaopen_base( lua );

		Cstr expansion[] = { variables... };
		// Take care of Named Return Value Optimization, which is faster than move semantics.
		std::unordered_map< Cstr, Value > retVals;
		for ( const auto it : expansion )
		{
			lua_pushstring( lua, it );
			lua_rawget( lua, LUA_RIDX_MAINTHREAD );
			// Equals lua_gettop( lua );
			const int index = -1;
			auto temp = lua_type( lua, index );
			switch ( temp )
			{
				case LUA_TBOOLEAN:
					retVals.emplace( it, static_cast<bool>(lua_toboolean(lua, index)) );
					break;
				case LUA_TNUMBER:
				// NOTE: Braces{} are placed to use a local variable 'number.'
				{
					const double number = lua_tonumber( lua, index );
					// When integer,
					if ( std::floor(number) == number )
					{
						// Exception: When either overflow or underflow occurs,
						if ( std::numeric_limits<int>::min() > number ||
							 std::numeric_limits<int>::max() < number )
						{
							const std::string msg( "Overflow or underflow occurs." );
#ifdef GAME_EXPORTS
							(*glpService).console( )->printFailure( FailureLevel::FATAL,
																			msg + it + scriptPathNName );
#else
							gService.console( )->printFailure( FailureLevel::FATAL,
																	  msg + it + scriptPathNName );
#endif

#ifdef _DEBUG
							__debugbreak( );
#else
							throw std::runtime_error( "" );
#endif
						}
						retVals.emplace( it, static_cast<int>(lua_tointeger(lua, index)) );
					}
					// When floating point number,
					else
					{
						// Exception: Double-precision at this point, but it'll be down-cast to single-precision.
						if ( std::numeric_limits<float>::min() > number ||
							 std::numeric_limits<float>::max() < number )
						{
							const std::string msg( "Overflow or underflow occurs." );
#ifdef GAME_EXPORTS
							(*glpService).console( )->printFailure( FailureLevel::FATAL,
																			msg + it + scriptPathNName );
#else
							gService.console( )->printFailure( FailureLevel::FATAL,
																	  msg + it + scriptPathNName );
#endif

#ifdef _DEBUG
							__debugbreak( );
#else
							throw std::runtime_error( "" );
#endif
						}
						retVals.emplace( it, static_cast<float>(number) );
					}
					break;
				}
				case LUA_TSTRING:
					retVals.emplace( it, std::string(lua_tostring(lua, index)) );
					break;
					// When one of variables itself doesn't exists or declared,
				case LUA_TNIL:
					break;
				default:
#ifdef _DEBUG
					__debugbreak( );
#else
					__assume( 0 );
#endif
			}
		}
		lua_close( lua );

		return retVals;
	}
}
