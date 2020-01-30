#include "../../pch.h"
#include "Waiting.h"
#include <Lib/ScriptLoader.h>
#include "Online.h"
#include "../../ServiceLocatorMirror.h"
#include "../VaultKeyList.h"
#include "../CommandList.h"

#include <iostream>

bool scene::online::Waiting::IsInstantiated = false;

scene::online::Waiting::Waiting( sf::RenderWindow& window, Online& net )
	: mOrder( 0 ),
	mState( ::scene::online::Waiting::State::TICKETING ),
	mWindow_( window ), mNet( net )
{
	ASSERT_TRUE( false == IsInstantiated );

	net.connectToQueueServer( );
	net.receive( );
	using WallClock = std::chrono::system_clock;
	const WallClock::time_point now = WallClock::now();
	const WallClock::duration tillNow = now.time_since_epoch();
	const std::chrono::minutes atMin = std::chrono::duration_cast<std::chrono::minutes>(tillNow);
	const HashedKey invitation = ::util::hash::Digest(VERSION + atMin.count() + SALT);
	Packet packet;
	packet.pack( TAG_INVITATION, invitation );
	net.send( packet );

	IsInstantiated = true;
}

scene::online::Waiting::~Waiting( )
{
	IsInstantiated = false;
}

void scene::online::Waiting::loadResources( )
{
	std::string font( "Fonts/AGENCYB.TTF" );
	sf::Vector2f centerPos( sf::Vector2f(mWindow_.getSize())*.5f );
	std::string label0Text( "Waiting" );
	uint32_t label0FontSize = 16;
	sf::Vector2f label0Position( centerPos.x, centerPos.y+50.f );
	uint32_t label1FontSize = 20;
	sf::Vector2f label1Position( centerPos.x-50.f, centerPos.y );
	std::string label2Text( "before me." );
	uint32_t label2FontSize = 16;
	sf::Vector2f label2Position( centerPos.x+50.f, centerPos.y );

	lua_State* lua = luaL_newstate();
	const std::string scriptPathNName( "Scripts/Waiting.lua" );
	if ( true == luaL_dofile(lua, scriptPathNName.data()) )
	{
		// File Not Found Exception
		gService( )->console( ).printFailure( FailureLevel::FATAL, std::string("File Not Found: ")+scriptPathNName );
	}
	else
	{
		luaL_openlibs( lua );
		const int TOP_IDX = -1;

		std::string varName( "Font" );
		lua_getglobal( lua, varName.data() );
		int type = lua_type(lua, TOP_IDX);
		if ( LUA_TSTRING == type )
		{
			font = lua_tostring(lua, TOP_IDX);
		}
		else if ( LUA_TNIL == type )
		{
			gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
												   varName, scriptPathNName );
		}
		else
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 varName, scriptPathNName );
		}
		lua_pop( lua, 1 );

		std::string tableName( "Label0" );
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			// Type Check Exception
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPathNName );
		}
		else
		{
			std::string field( "msg" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TSTRING == type )
			{
				label0Text = lua_tostring(lua, TOP_IDX);
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "fontSize";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				label0FontSize = (uint32_t)lua_tointeger(lua, TOP_IDX);
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "x";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				label0Position.x = (float)lua_tonumber(lua, TOP_IDX);
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "y";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				label0Position.y = (float)lua_tonumber(lua, TOP_IDX);
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );
		}
		lua_pop( lua, 1 );

		tableName = "Label1";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPathNName );
		}
		else
		{
			std::string field( "fontSize" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				label1FontSize = (uint32_t)lua_tointeger(lua, TOP_IDX);
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "x";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				label1Position.x = (float)lua_tonumber(lua, TOP_IDX);
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "y";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				label1Position.y = (float)lua_tonumber(lua, TOP_IDX);
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														tableName+":"+field, scriptPathNName );
			}
			lua_pop( lua, 1 );
		}
		lua_pop( lua, 1 );

		tableName = "Label2";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPathNName );
		}
		else
		{
			std::string field( "msg" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TSTRING == type )
			{
				label2Text = lua_tostring(lua, TOP_IDX);
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "fontSize";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				label2FontSize = (uint32_t)lua_tointeger(lua, TOP_IDX);
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "x";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				label2Position.x = (float)lua_tonumber(lua, TOP_IDX);
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "y";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				label2Position.y = (float)lua_tonumber(lua, TOP_IDX);
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );
		}
		lua_pop( lua, 1 );
	}
	lua_close( lua );

	mFont.loadFromFile( font );
	mTextLabels[0].setFont( mFont );
	mTextLabels[0].setString( label0Text );
	mTextLabels[0].setCharacterSize( label0FontSize );
	const sf::FloatRect bound0 = mTextLabels[0].getLocalBounds();
	mTextLabels[0].setPosition( label0Position );
	mTextLabels[1].setFont( mFont );
	mTextLabels[1].setString( std::to_string(mOrder) );
	mTextLabels[1].setCharacterSize( label1FontSize );
	const sf::FloatRect bound1 = mTextLabels[1].getLocalBounds();
	mTextLabels[1].setPosition( label1Position );
	mTextLabels[2].setFont( mFont );
	mTextLabels[2].setCharacterSize( label2FontSize );
	mTextLabels[2].setString( label2Text );
	const sf::FloatRect bound2 = mTextLabels[2].getLocalBounds();
	mTextLabels[2].setPosition( label2Position );
}

::scene::online::ID scene::online::Waiting::update( std::vector<sf::Event>& eventQueue )
{
	::scene::online::ID retVal = ::scene::online::ID::AS_IS;
	switch ( mState )
	{
		case State::TICKETING:
			if ( true == mNet.hasReceived() )
			{
				if ( std::optional<std::string> ticket( mNet.getByTag(TAG_TICKET,
																	   Online::Option::RETURN_TAG_ATTACHED,
																	   sizeof(uint32_t)) );
					 std::nullopt != ticket )
				{
					std::string& _ticket = ticket.value();
#ifdef _DEBUG
					const char* const ptr = _ticket.data();

					std::string ticketID( "ticket: "+std::to_string(::ntohl(*(HashedKey*)&ptr[std::strlen(TAG_TICKET)])) );
					gService( )->console( ).print( ticketID, sf::Color::Green );
#endif
					if ( true == mNet.connectToMainServer() )
					{
						// Sending to the main server the ticket, which the main server will verify.
						mNet.send( _ticket.data(), (int)_ticket.size() );
						mState = State::SUBMITTING_TICKET;
					}
				}
				else if ( std::optional<std::string> order( mNet.getByTag(TAG_ORDER_IN_QUEUE,
																		 Online::Option::FIND_END_TO_BEGIN,
																		   sizeof(uint32_t)) );
						  std::nullopt != order )
				{
					mOrder = ::ntohl(*(uint32_t*)order.value().data());
					const std::string _order( std::to_string(mOrder) );
					mTextLabels[1].setString( _order );
				}
				else
				{
					gService( )->console( ).printFailure( FailureLevel::FATAL,
														 "Unknown message from the queue server." );
					mNet.disconnect( );
					break;
				}
				mNet.receive( );
			}
			break;
		case State::SUBMITTING_TICKET:
			if ( true == mNet.hasReceived() )
			{
				if ( std::optional<std::string> nickname( mNet.getByTag(TAG_MY_NICKNAME,
																		Online::Option::DEFAULT,
																		-1) );
					 std::nullopt != nickname )
				{
#ifdef _DEBUG
					gService( )->console( ).print( nickname.value(), sf::Color::Green );
#endif
					mNet.setMyNickname( nickname.value() );
					retVal = ::scene::online::ID::IN_LOBBY;
				}
				else
				{
					mNet.disconnect( );
				}
			}
			break;
		default:
#ifdef _DEBUG
			__debugbreak( );
#else
			__assume(0);
#endif
			break;
	}
	for ( auto it = eventQueue.cbegin(); eventQueue.cend() !=it; )
	{
		if ( sf::Event::KeyPressed == it->type &&
			sf::Keyboard::Escape == it->key.code )
		{
			retVal = ::scene::online::ID::MAIN_MENU;
			it = eventQueue.erase(it);
		}
		else
		{
			++it;
		}
	}
	return retVal;
}

void scene::online::Waiting::draw( )
{
	for ( sf::Text& textLabel : mTextLabels )
	{
		mWindow_.draw( textLabel );
	}
}