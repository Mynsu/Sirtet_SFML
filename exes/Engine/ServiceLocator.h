#pragma once
#pragma hdrstop
#include <memory>
#include <unordered_map>
#include <variant>
#include <string_view>
#include <type_traits>
#include <intrin.h> // __debugbreak( )
#include <Lib/Hash.h>
#include <SFML/System.hpp>
#include <lua.hpp>
#include "Console.h"

using Dword = int32_t;

class ServiceLocator
{
public:
	// Get console.
	static auto Console( ) -> std::unique_ptr< IConsole >&
	{
		return _Console;
	}
	// Initialize and get console.
	static auto ConsoleWithInit( const sf::Vector2u& winSize ) -> std::unique_ptr< IConsole >& //TODO:�̸� ��..
	{
		_Console->init( winSize );
		return _Console;
	}
	static auto VariableTable( ) -> std::unordered_map< HashedKey, Dword >&
	{
		return _VariableTable;
	}
	// Load data from a .lua script file.
	// Only is 'std::string' type allowed, neither 'const char*' nor 'char[]' type allowed.
	// Value type is now 'std::variant< bool, int, float, std::string >, which you can customize.
	// This throws std::runtime_error when failing to open the script file.
	template < typename Value = std::variant< bool, int, float, std::string >, typename Str, typename... Strs,
		std::enable_if_t< std::is_same_v< std::decay_t< Str >, std::string > >* = nullptr >
	static const std::unordered_map< std::string, Value > LoadFromScript( const Str& scriptPathNName, const Strs&... variables )//TODO: ��Ű���� ���� �� ��ȣȭ
	{
		lua_State* lua = luaL_newstate( );
		//TODO
		///luaopen_base( lua );
		// Exception handling
		// Make sure that this function returns true on failure.
		if ( true == luaL_dofile( lua, scriptPathNName.data( ) ) )
		{
			lua_close( lua );
			//�ñ�: �� ���� ��� ������?
			::global::Console( )->printError( ErrorLevel::CRITICAL, "File not found: " + scriptPathNName );
#ifdef _DEBUG
			__debugbreak( );
#elif
			throw std::runtime_error;
#endif
		}
		luaopen_base( lua );

		const std::string_view expansion[] = { variables... };
		// Take care of Named Return Value Optimization.
		std::unordered_map< Str, Value > retVals;
		for ( const auto& it : expansion )
		{
			lua_pushstring( lua, it.data( ) );
			lua_rawget( lua, LUA_RIDX_MAINTHREAD );
			const int index = -1; //TODO ///lua_gettop( lua );
			auto temp = lua_type( lua, index );
			switch ( temp )
			{
				case LUA_TBOOLEAN:
					retVals.emplace( it, static_cast< bool >( lua_toboolean( lua, index ) ) );
					break;
				case LUA_TNUMBER:
				// NOTE: Braces{} are placed in order to use a local variable 'number.'
				{
					const double number = lua_tonumber( lua, index );
					// When integer,
					if ( std::floor( number ) == number )
					{
						// When either overflow or underflow occurs,
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
					// When floating point number,
					else
					{
						// Double-precision at this point, but it'll be down-cast to single-precision.
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
					// When one of variables itself doesn't exists or declared,
				case LUA_TNIL:
					break;
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
	static std::unique_ptr< IConsole > _Console;//TODO: �ܼ��� ���߿����θ� �ѱ�, �ֿܼ� ���� ������ �ѱ�?
	static std::unordered_map< HashedKey, Dword > _VariableTable;
};	