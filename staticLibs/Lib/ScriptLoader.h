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
	// Value type is std::variant<bool, int, float, std::string>, which you can customize.
	// This throws std::runtime_error when failing to open the script file.
	template <typename Value=std::variant<bool,int,float,std::string>, typename Str, typename... Strs,
		std::enable_if_t<std::is_same_v<std::decay_t<Str>,std::string>>* = nullptr>
		static const std::unordered_map<Str, Value> LoadFromScript( Str scriptPath, Strs... variables )
	{
		std::unordered_map<Str, Value> retVals;
		lua_State* lua = luaL_newstate();
		if ( true == luaL_dofile(lua, scriptPath.data()) )
		{
			lua_close( lua );
#ifdef GAME_EXPORTS
			gService()->console().printFailure( FailureLevel::FATAL, "File Not Found: "+scriptPath );
#else
			gService._console().printFailure( FailureLevel::FATAL, "File Not Found: "+scriptPath );
#endif
			return retVals;
		}
		luaopen_base( lua );

		Str expansion[] = { variables... };
		for ( const auto str : expansion )
		{
			const int TOP_IDX = -1;
			lua_getglobal( lua, str.data() );
			int type = lua_type(lua, TOP_IDX);
			switch ( type )
			{
				case LUA_TBOOLEAN:
					retVals.emplace( str, (bool)lua_toboolean(lua, TOP_IDX) );
					break;
				case LUA_TNUMBER:
				{
					const double number = lua_tonumber(lua, TOP_IDX);
					// When integer,
					if ( std::floor(number) == number )
					{
						retVals.emplace( str, (int)lua_tointeger(lua, TOP_IDX) );
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
							gService()->console().printFailure( FailureLevel::WARNING,
																			msg + str + scriptPath );
#else
							gService._console().printFailure( FailureLevel::WARNING,
																	  msg + str + scriptPath );
#endif
							continue;
						}
						retVals.emplace( str, (float)number );
					}
					break;
				}
				case LUA_TSTRING:
					retVals.emplace( str, std::string(lua_tostring(lua, TOP_IDX)) );
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
					break;
			}
		}
		lua_close( lua );

		return retVals;
	}
}
