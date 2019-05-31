#pragma once
#pragma hdrstop
#include <memory>
#include <vector>
#include <unordered_map>
#include <variant>
#include <string_view>
#include <type_traits>
#include <intrin.h> // __debugbreak( ), int32_t
#include <SFML/System.hpp>
#include <lua.hpp>
#include "Console.h"

using hashValue_t = int32_t;
using dword = int32_t;

class ServiceLocator
{
public:
	// Get console.
	static auto Console( ) -> std::unique_ptr< IConsole >&
	{
		return _Console;
	}
	// Initialize and get console.
	static auto Console( const sf::Vector2u& winSize ) -> std::unique_ptr< IConsole >&
	{
		_Console->init( winSize );
		return _Console;
	}
	static auto VariableTable( ) -> std::unordered_map< hashValue_t, dword >&
	{
		return variableTable;
	}
	// Load data from a .lua script file.  Thus, this can throw std::bad_exception when failed to open the script file.
	// Returns an immutable hash table 'const std::unordered_map< std::string, typename Value >.'
	//		Only is 'std::string' type allowed to function arguments for security, neither 'const char*' nor 'char[]' type allowed.
	//		Otherwise, a complier cannot deduce or substitute this function template thanks to SFINAE.
	//		Put in one or more variable identifiers as such on a script file as the second function argument pack.
	// The first template argument 'Value' is that of std::pair< Key, Value > in hash table.
	// Default is 'std::variant< bool, int, float, std::string >, which you can customize.
	template < typename Value = std::variant< bool, int, float, std::string >, typename Str, typename... Strs,
		std::enable_if_t< std::is_same_v< std::decay_t< Str >, std::string > >* = nullptr >///,
		///std::enable_if_t< std::is_>
	static const std::unordered_map< std::string, Value > LoadFromScript( const Str& scriptPathNName, const Strs&... variables )//TODO: 패키지에 모은 후 파일명 암호화
	{
		lua_State* lua = luaL_newstate( );
		luaopen_base( lua );
		// Exception handling
		if ( true == luaL_dofile( lua, scriptPathNName.data( ) ) )
		{
			lua_close( lua );
			_Console->printError( "Failed to load " + scriptPathNName );
#ifdef _DEBUG
			__debugbreak( );
#elif
			throw std::runtime_error;
#endif
		}

		const std::string_view expansion[] = { variables... };
		// Take care of Named Return Value Optimization.
		std::unordered_map< Str, Value > retVals;
		for ( const auto& it : expansion )
		{
			lua_pushstring( lua, it.data( ) );
			lua_rawget( lua, LUA_RIDX_MAINTHREAD );
			const int index = lua_gettop( lua );
			switch ( lua_type( lua, index ) )
			{
				case LUA_TBOOLEAN:
					retVals.emplace( it, static_cast< bool >( lua_toboolean( lua, index ) ) );
					break;
				case LUA_TNUMBER:
				// NOTE: Braces{} are placed so as to use a local variable 'number.'
				{
					const double number = lua_tonumber( lua, index );
					if ( std::floor( number ) == number )
					{
						if ( std::numeric_limits< int >::min() > number ||
							 std::numeric_limits< int >::max() < number )
						{
#ifdef _DEBUG
							__debugbreak( );
#elif
							throw std::runtime_error;
#endif
						}
						retVals.emplace( it, static_cast< int >( lua_tointeger( lua, index ) ) );
					}
					else
					{
						if ( std::numeric_limits< float >::min() > number ||
							 std::numeric_limits< float >::max() < number )
						{
#ifdef _DEBUG
							__debugbreak( );
#elif
							throw std::runtime_error;
#endif
						}
						retVals.emplace( it, static_cast< float >( number ) );
					}
					break;
				}
				case LUA_TSTRING:
					retVals.emplace( it, std::move( std::string( lua_tostring( lua, index ) ) ) );
					break;
				//TODO: case LUA_TTABLE:
				default:
#ifdef _DEBUG
					__debugbreak( );
#elif
					__assume( 0 );
#endif
			}
		}
		lua_close( lua );

		return retVals;
	}
private:
	static std::unique_ptr< IConsole > _Console;//TODO: 콘솔을 개발용으로만 둘까, 콘솔에 유저 권한을 둘까?
	static std::unordered_map< hashValue_t, dword > variableTable;
};	