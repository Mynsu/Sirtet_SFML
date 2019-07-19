#include "Playing.h"
#include <lua.hpp>

struct PanelPosNSize
{
	uint16_t x, y, width, height;
};

sequence::inPlay::Playing::Playing( sf::RenderWindow& window,
									::sequence::inPlay::Seq* const nextInPlaySequence,
									sf::Drawable& shapeOrSprite )
	: mWindow( window ), mNextInPlaySequence( nextInPlaySequence ),
	mBackgroundRect( static_cast< sf::RectangleShape& >( shapeOrSprite ) )
{
	// Cyan
	const uint32_t BACKGROUND_RGB = 0x29cdb5ffu;
	mBackgroundRect.setFillColor( sf::Color( BACKGROUND_RGB ) );

	//TODO: array<array<string>>
	//TODO: console ¸í·É¾î refresh

	lua_State* lua = luaL_newstate( );
	const std::string scriptPathNName( "Scripts/Playing.lua" );
	if ( true == luaL_dofile( lua, scriptPathNName.data( ) ) )
	{
		::global::Console( )->printError( ErrorLevel::WARNING, "File not found: " + scriptPathNName );
		lua_close( lua );
	}
	luaL_openlibs( lua );
	const std::string tableName( "PlayerPanel" );
	lua_getglobal( lua, tableName.data(	) );
	PanelPosNSize temp = { 100, 100, 300, 400 };
	if ( false == lua_istable( lua, -1 ) )
	{
		::global::Console( )->printScriptError( ErrorLevel::WARNING,
												tableName, tableName + " in " + scriptPathNName + " is not table type." );
	}
	else
	{
		lua_pushstring( lua, "x" );
		lua_gettable( lua, 1 );
		temp.x = static_cast< uint16_t >( lua_tointeger( lua, -1 ) );
		lua_pop( lua, 1 );

		lua_pushstring( lua, "y" );
		lua_gettable( lua, 1 );
		temp.y = static_cast< uint16_t >( lua_tointeger( lua, -1 ) );
		lua_pop( lua, 1 );

		lua_pushstring( lua, "width" );
		lua_gettable( lua, 1 );
		temp.width = static_cast< uint16_t >( lua_tointeger( lua, -1 ) );
		lua_pop( lua, 1 );

		lua_pushstring( lua, "height" );
		lua_gettable( lua, 1 );
		temp.height = static_cast< uint16_t >( lua_tointeger( lua, -1 ) );
		lua_pop( lua, 1 );
	}
	lua_close( lua );

	mPlayerPanel.setPosition( sf::Vector2f( temp.x, temp.y ) );
	mPlayerPanel.setSize( sf::Vector2f( temp.width, temp.height ) );
	mPlayerPanel.setFillColor( sf::Color::Black );
}

void sequence::inPlay::Playing::draw( )
{
	mWindow.draw( mBackgroundRect );
	mWindow.draw( mPlayerPanel );
}
