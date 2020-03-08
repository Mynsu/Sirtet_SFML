#include "../../pch.h"
#include "Waiting.h"
#include "Online.h"
#include "../../ServiceLocatorMirror.h"

bool scene::online::Waiting::IsInstantiated = false;

scene::online::Waiting::Waiting( const sf::RenderWindow& window, Online& net )
	: mOrder( 0 ),
	mState( ::scene::online::Waiting::State::TICKETING )
{
	ASSERT_TRUE( false == IsInstantiated );

	loadResources( window );
	net.connectToQueueServer( );
	net.receive( );
	using WallClock = std::chrono::system_clock;
	const WallClock::time_point now = WallClock::now();
	const std::chrono::seconds atSec =
		std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
	const uint32_t seed = (uint32_t)atSec.count();
	std::minstd_rand engine( seed + SALT );
	engine.discard( engine() % MAX_KEY_STRETCHING );
	const uint32_t invitation = engine();
	Packet packet;
	packet.pack( TAG_INVITATION, invitation );
	net.send( packet );

	IsInstantiated = true;
}

scene::online::Waiting::~Waiting( )
{
	IsInstantiated = false;
}

void scene::online::Waiting::loadResources( const sf::RenderWindow& window )
{
	std::string font( "Fonts/AGENCYB.TTF" );
	sf::Vector2f centerPos( sf::Vector2f(window.getSize())*.5f );
	std::string label0Text( "Waiting" );
	uint16_t label0FontSize = 16;
	sf::Vector2f label0Position( centerPos.x, centerPos.y+50.f );
	uint16_t label1FontSize = 20;
	sf::Vector2f label1Position( centerPos.x-50.f, centerPos.y );
	std::string label2Text( "before me." );
	uint16_t label2FontSize = 16;
	sf::Vector2f label2Position( centerPos.x+50.f, centerPos.y );
	mSoundPaths[(int)SoundIndex::SELECTION] = "Sounds/selection.wav";

	lua_State* lua = luaL_newstate();
	const std::string scriptPath( "Scripts/Waiting.lua" );
	if ( true == luaL_dofile(lua, scriptPath.data()) )
	{
		gService()->console().printFailure( FailureLevel::FATAL,
										  "File Not Found: "+scriptPath );
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
												   varName, scriptPath );
		}
		else
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													 varName, scriptPath );
		}
		lua_pop( lua, 1 );

		std::string tableName( "Label0" );
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPath );
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
													   tableName+':'+field, scriptPath );
			}
			else
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPath );
			}
			lua_pop( lua, 1 );

			field = "fontSize";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				label0FontSize = (uint16_t)lua_tointeger(lua, TOP_IDX);
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPath );
			}
			else
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPath );
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
													   tableName+':'+field, scriptPath );
			}
			else
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPath );
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
													   tableName+':'+field, scriptPath );
			}
			else
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPath );
			}
			lua_pop( lua, 1 );
		}
		lua_pop( lua, 1 );

		tableName = "Label1";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPath );
		}
		else
		{
			std::string field( "fontSize" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				label1FontSize = (uint16_t)lua_tointeger(lua, TOP_IDX);
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPath );
			}
			else
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPath );
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
													   tableName+':'+field, scriptPath );
			}
			else
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPath );
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
													   tableName+':'+field, scriptPath );
			}
			else
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
														tableName+":"+field, scriptPath );
			}
			lua_pop( lua, 1 );
		}
		lua_pop( lua, 1 );

		tableName = "Label2";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPath );
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
													   tableName+':'+field, scriptPath );
			}
			else
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPath );
			}
			lua_pop( lua, 1 );

			field = "fontSize";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				label2FontSize = (uint16_t)lua_tointeger(lua, TOP_IDX);
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPath );
			}
			else
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													tableName+':'+field, scriptPath );
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
													   tableName+':'+field, scriptPath );
			}
			else
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPath );
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
													   tableName+':'+field, scriptPath );
			}
			else
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPath );
			}
			lua_pop( lua, 1 );
		}
		lua_pop( lua, 1 );

		tableName = "Sound";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK, tableName, scriptPath );
		}
		else
		{
			std::string innerTableName( "onSelection" );
			lua_pushstring( lua, innerTableName.data() );
			lua_gettable( lua, 1 );
			if ( false == lua_istable(lua, TOP_IDX) )
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+innerTableName, scriptPath );
			}
			else
			{
				std::string field( "path" );
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				int type = lua_type(lua, TOP_IDX);
				if ( LUA_TSTRING == type )
				{
					mSoundPaths[(int)SoundIndex::SELECTION] = lua_tostring(lua, TOP_IDX);
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+field, scriptPath );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+field, scriptPath );
				}
				lua_pop( lua, 1 );
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

::scene::online::ID scene::online::Waiting::update( std::vector<sf::Event>& eventQueue,
												   ::scene::online::Online& net,
												   const sf::RenderWindow& )
{
	::scene::online::ID retVal = ::scene::online::ID::AS_IS;
	switch ( mState )
	{
		case State::TICKETING:
			if ( true == net.hasReceived() )
			{
				if ( std::optional<std::string> ticket( net.getByTag(TAG_TICKET,
																	Online::Option::RETURNING_TAG_ATTACHED,
																	sizeof(HashedKey)) );
					 std::nullopt != ticket )
				{
					std::string& _ticket = ticket.value();
#ifdef _DEBUG
					const char* const ptr = _ticket.data();

					std::string ticketID( "ticket: "+std::to_string(::ntohl(*(HashedKey*)&ptr[std::strlen(TAG_TICKET)])) );
					gService()->console().print( ticketID, sf::Color::Green );
#endif
					if ( true == net.connectToMainServer() )
					{
						// NOTE: 대기열 서버가 메인 서버에 보낸 티켓이 도착한 후에 클라이언트에게 티켓을 보내는 방법이 더 세련되지만
						// 대기열 서버와 메인 서버에 걸리는 부하에 비하면 괜한 일을 하는 것일 수 있습니다.
						// 그래서 이렇게 클라이언트가 기다렸다가 메인 서버에 티켓을 에코하도록 했습니다.
						std::thread sendingTicket( [&net, _ticket]() mutable
												  {
													std::this_thread::sleep_for( std::chrono::milliseconds(1000) );
													net.send( _ticket.data(), (int)_ticket.size() );
												  } );
						sendingTicket.detach();
						mState = State::SUBMITTING_TICKET;
					}
				}
				else if ( std::optional<std::string> order( net.getByTag(TAG_ORDER_IN_QUEUE,
																		 Online::Option::FINDING_END_TO_BEGIN,
																		   sizeof(uint16_t)) );
						  std::nullopt != order )
				{
					mOrder = ::ntohs(*(uint16_t*)order.value().data());
					const std::string _order( std::to_string(mOrder) );
					mTextLabels[1].setString( _order );
				}
				else
				{
					gService()->console().printFailure( FailureLevel::FATAL,
														 "Unknown message from the queue server." );
					net.disconnect( );
					return retVal;
				}
				net.receive( );
			}
			break;
		case State::SUBMITTING_TICKET:
			if ( true == net.hasReceived() )
			{
				if ( std::optional<std::string> nickname( net.getByTag(TAG_MY_NICKNAME,
																		Online::Option::DEFAULT,
																		-1) );
					 std::nullopt != nickname )
				{
#ifdef _DEBUG
					gService()->console().print( nickname.value(), sf::Color::Green );
#endif
					net.setMyNickname( nickname.value() );
					retVal = ::scene::online::ID::IN_LOBBY;
				}
				else
				{
					net.disconnect( );
				}
			}
			break;
		default:
#ifdef _DEBUG
			__debugbreak( );
#else
			__assume(0);
#endif
	}
	for ( auto it = eventQueue.cbegin(); eventQueue.cend() !=it; )
	{
		if ( sf::Event::KeyPressed == it->type &&
			sf::Keyboard::Escape == it->key.code )
		{
			if ( false == gService()->sound().playSFX(mSoundPaths[(int)SoundIndex::SELECTION]) )
			{
				gService()->console().printFailure(FailureLevel::WARNING,
												   "File Not Found: "+mSoundPaths[(int)SoundIndex::SELECTION] );
			}
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

void scene::online::Waiting::draw( sf::RenderWindow& window )
{
	for ( sf::Text& textLabel : mTextLabels )
	{
		window.draw( textLabel );
	}
}