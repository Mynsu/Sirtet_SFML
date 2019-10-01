#include "../../pch.h"
#include "Lobby.h"
#include "../../ServiceLocatorMirror.h"
#include <lua.hpp>

bool ::scene::online::Lobby::IsInstantiated = false;

scene::online::Lobby::Lobby( sf::RenderWindow& window, const SetScene_t& setScene )
	: mFrameCount_disconnection( 0u ), mQueueNumber( MAXINT32 ),
	mSocketToQueueServer( std::make_unique<Socket>(Socket::Type::TCP) ),
	mSocketToMainServer( std::make_unique<Socket>(Socket::Type::TCP) ),
	mWindow_( window ), mSetScene( setScene )
{
	ASSERT_FALSE( IsInstantiated );

	constexpr HashedKey HK_FORE_FPS = ::util::hash::Digest( "foreFPS", 7 );
	mFPS_ = static_cast<uint32_t>((*glpService).vault( )[ HK_FORE_FPS ]);

	ASSERT_TRUE( -1 != mSocketToQueueServer->bind(EndPoint::Any) );
	if ( char queueServerIPAddress[ ] = "192.168.219.102";
		 -1 == mSocketToQueueServer->connect(EndPoint(queueServerIPAddress, 10000)) )
	{
		// Exception
		(*glpService).console( )->printFailure( FailureLevel::WARNING, "Connection to queue server failed.\n" );
		// Triggering
		++mFrameCount_disconnection;
	}
	else
	{
		(*glpService).console( )->print( "Connection to queue server succeeded.", sf::Color::Green );
		// Awaiting 
		char* const rcvBuf = mSocketToQueueServer->receivingBuffer( );
		if ( -1 == ::recv(mSocketToQueueServer->handle(),
						   rcvBuf, mSocketToQueueServer->MAX_RCV_BUF_LEN, 0) )
		{
			// Exception
			(*glpService).console( )->printFailure( FailureLevel::WARNING, "Receiving from queue server failed.\n" );
		}
		const uint8_t qTTagLen = 3u;
		const HashedKey refinedQTTag = ::util::hash::Digest( "qT:", qTTagLen );
		if ( refinedQTTag == ::util::hash::Digest(rcvBuf, qTTagLen) )
		{
			mQueueNumber = std::atoi( &rcvBuf[qTTagLen] );
#ifdef _DEBUG
			std::string msg( "Received a queue ticket: " );
			(*glpService).console( )->print( msg + rcvBuf, sf::Color::Green );
#endif
		}
		else
		{
			ASSERT_TRUE( -1 != mSocketToMainServer->bind(EndPoint::Any) );
			if ( char mainServerIPAddress[ ] = "192.168.219.102";
				 -1 == mSocketToMainServer->connect(EndPoint(mainServerIPAddress, 54321)) )
			{
				// Exception
				(*glpService).console( )->printFailure( FailureLevel::WARNING, "Connection to main server failed.\n" );
				// Triggering
				++mFrameCount_disconnection;
			}
			else
			{
				if ( -1 == ::send(mSocketToMainServer->handle(),
								   rcvBuf, (int)std::strlen(rcvBuf)+1, 0) )
				{
					// Exception
					(*glpService).console( )->printFailure( FailureLevel::WARNING, "Sending the ticket to the main server failed.\n" );
					// Triggering
					++mFrameCount_disconnection;
				}
				else
				{
					(*glpService).console( )->print( "Connection to main server succeeded.", sf::Color::Green );
				}
			}
		}
	}
	loadResources( );

	IsInstantiated = true;
}

::scene::online::Lobby::~Lobby( )
{
	mSocketToQueueServer->close( );
	mSocketToMainServer->close( );

	IsInstantiated = false;
}

void ::scene::online::Lobby::loadResources( )
{
	bool isPathDefault = true;
	bool isWDefault = true;
	bool isHDefault = true;
	lua_State* lua = luaL_newstate( );
	char scriptPathNName[] = "Scripts/Lobby.lua";
	if ( true == luaL_dofile(lua, scriptPathNName) )
	{
		// File Not Found Exception
		(*glpService).console( )->printFailure( FailureLevel::FATAL, std::string("File Not Found: ")+scriptPathNName );
		lua_close( lua );
	}
	else
	{
		luaL_openlibs( lua );
		const int TOP_IDX = -1;
		const std::string tableName0( "Sprite" );
		lua_getglobal( lua, tableName0.data( ) );
		// Type Check Exception
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName0.data( ), scriptPathNName );
		}
		else
		{
			const char field0[ ] = "path";
			lua_pushstring( lua, field0 );
			lua_gettable( lua, 1 );
			int type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TSTRING == type )
			{
				if ( false == mTexture.loadFromFile(lua_tostring(lua, TOP_IDX)) )
				{
					// File Not Found Exception
					(*glpService).console( )->printScriptError( ExceptionType::FILE_NOT_FOUND,
						(tableName0+":"+field0).data( ), scriptPathNName );
				}
				else
				{
					isPathDefault = false;
				}
			}
			// Type Check Exception
			else
			{
				(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK,
					(tableName0+":"+field0).data( ), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field1[ ] = "width";
			lua_pushstring( lua, field1 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				const float temp = static_cast<float>(lua_tointeger(lua, TOP_IDX));
				// Range Check Exception
				if ( 0 > temp )
				{
					(*glpService).console( )->printScriptError( ExceptionType::RANGE_CHECK,
						(tableName0+":"+field1).data( ), scriptPathNName );
				}
				// When the value looks OK,
				else
				{
					mSpriteClipSize_.x = temp;
					isWDefault = false;
				}
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK,
					(tableName0+":"+field1).data( ), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field2[ ] = "height";
			lua_pushstring( lua, field2 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				const float temp = static_cast<float>(lua_tointeger(lua, TOP_IDX));
				// Range Check Exception
				if ( 0 > temp )
				{
					(*glpService).console( )->printScriptError( ExceptionType::RANGE_CHECK,
						(tableName0+":"+field2).data( ), scriptPathNName );
				}
				// When the value looks OK,
				else
				{
					mSpriteClipSize_.y = temp;
					isHDefault = false;
				}
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK,
					(tableName0+":"+field2).data( ), scriptPathNName );
			}
			lua_pop( lua, 2 );
		}
		lua_close( lua );
	}

	if ( true == isPathDefault )
	{
		const char defaultFilePathNName[ ] = "Images/Lobby.png";
		if ( false == mTexture.loadFromFile(defaultFilePathNName) )
		{
			// Exception: When there's not even the default file,
			(*glpService).console( )->printFailure( FailureLevel::FATAL, std::string( "File Not Found: " )+defaultFilePathNName );
#ifdef _DEBUG
			__debugbreak( );
#endif
		}
	}

	if ( true == isWDefault || true == isHDefault )
	{
		(*glpService).console( )->print( "Default: width 256, height 128" );
	}

	mSprite.setTexture( mTexture );
	mSprite.setPosition( (sf::Vector2f(mWindow_.getSize( ))-mSpriteClipSize_)*0.5f );
	const sf::Vector2i cast( mSpriteClipSize_ );
	if ( 0 != mFrameCount_disconnection )
	{
		mSprite.setTextureRect( sf::IntRect(0, cast.y, cast.x, cast.y) );
	}
	else
	{
		mSprite.setTextureRect( sf::IntRect(0, 0, cast.x, cast.y) );
	}
}

void ::scene::online::Lobby::update( std::list<sf::Event>& eventQueue )
{
	if ( 3*mFPS_ < mFrameCount_disconnection )
	{
		mSetScene( ::scene::ID::MAIN_MENU );
		return;
	}

	char* rcvBuf = mSocketToQueueServer->receivingBuffer( );
	if ( MAXINT32 != mQueueNumber )
	{
		if ( 0 < mQueueNumber )
		{
			if ( -1 == ::recv(mSocketToQueueServer->handle(), rcvBuf,
							  mSocketToQueueServer->MAX_RCV_BUF_LEN, 0) )
			{
				// Exception
				(*glpService).console( )->printFailure( FailureLevel::WARNING, "Receiving from queue server failed.\n" );
			}
			else
			{
				mQueueNumber -= std::atoi( &rcvBuf[4] );
#ifdef _DEBUG
				std::string msg( "Received the shorter queue number: " );
				(*glpService).console( )->print( msg + rcvBuf, sf::Color::Green );
#endif
			}
		}
		else
		{
			ASSERT_TRUE( -1 != mSocketToMainServer->bind(EndPoint::Any) );
			if ( char mainServerIPAddress[ ] = "192.168.219.102";
				 -1 == mSocketToMainServer->connect(EndPoint(mainServerIPAddress, 54321)) )
			{
				// Exception
				(*glpService).console( )->printFailure( FailureLevel::WARNING, "Connection to main server failed.\n" );
				// Triggering
				++mFrameCount_disconnection;
			}
			else
			{
				if ( -1 == ::send(mSocketToMainServer->handle( ), rcvBuf, (int)std::strlen(rcvBuf)+1, 0) )
				{
					// Exception
					(*glpService).console( )->printFailure( FailureLevel::WARNING, "Sending the ticket to the main server failed.\n" );
					// Triggering
					++mFrameCount_disconnection;
				}
				else
				{
					(*glpService).console( )->print( "Connection to main server succeeded.", sf::Color::Green );
				}
			}
		}
	}

	//char sndBuf[ ] = "Testing Testing";
	//const int sndLen = ::send( (*glpService).socket( ).handle( ),
	//						   sndBuf,
	//						   (int)::strlen( sndBuf )+1,
	//						   0 );
	// Exception: SOCKET_ERROR -1
	//if ( sndLen < 0 )
	//{
	//	(*glpService).console()->printFailure( ::FailureLevel::FATAL, "Sending failed." );
	//}

	//Socket& socket = (*glpService).socket( );
	//const int rcvLen = ::recv( socket.handle(),
	//						   socket.receivingBuffer(),
	//						   socket.MAX_RCV_BUF_LEN,
	//						   0 );
	//if ( rcvLen > 0 )
	//{
	//	//TODO
	//	(*glpService).console()->print( socket.receivingBuffer( ), sf::Color::Green );
	//}
	//// Exception: SOCKET_ERROR -1
	//else if ( rcvLen < 0 )
	//{
	//	(*glpService).console()->printFailure( ::FailureLevel::FATAL, "Receiving failed." );
	//}

}

void ::scene::online::Lobby::draw( )
{
	mWindow_.draw( mSprite );
	if ( 0u != mFrameCount_disconnection )
	{
		++mFrameCount_disconnection;
	}
}

::scene::ID scene::online::Lobby::currentScene( ) const
{
	return ::scene::ID::ONLINE_BATTLE;//TODO
}
