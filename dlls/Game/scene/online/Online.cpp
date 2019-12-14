#include "../../pch.h"
#include "Online.h"
#include <lua.hpp>
#include "../../ServiceLocatorMirror.h"
#include "../VaultKeyList.h"
#include "Waiting.h"
#include "InLobby.h"
#include "InRoom.h"

bool ::scene::online::Online::IsInstantiated = false;

namespace
{
	const uint8_t SECONDS_TO_MAIN_MENU = 3u;
	std::unique_ptr< Socket > SocketToServer;
//TODO: 개명
	std::unique_ptr< std::thread > ThreadToReceive;
	bool IsReceiving;
	int ReceivingResult;
	int Error;
	std::condition_variable CvForResumingRcv;
	std::mutex MutexRcvBuf;
	uint32_t FrameCount_interval = 0;

	void Receive( Socket& socket )
	{
		IsReceiving = true;
		while ( true == IsReceiving )
		{
			ReceivingResult = socket.receiveBlock();
			Error =	WSAGetLastError();
			if ( -1 == ReceivingResult && WSAETIMEDOUT == Error )
			{
				// Trying to receive again
			}
			else if ( 0 < ReceivingResult )
			{
				std::unique_lock uLock( MutexRcvBuf );
				CvForResumingRcv.wait( uLock );
			}
			else
			{
				break;
			}
		}
	}
}

scene::online::Online::Online( sf::RenderWindow& window )
	: mFrameCount_disconnection( 0u ),
	mWindow_( window )
{
	ASSERT_FALSE( IsInstantiated );

	mFPS_ = (uint32_t)gService( )->vault( )[ HK_FORE_FPS ];
	FrameCount_interval = mFPS_;
	SocketToServer = std::make_unique< Socket >( Socket::Type::TCP );
	ASSERT_TRUE( -1 != SocketToServer->bind(EndPoint::Any) );
	// NOTE: Setting socket option should be done following binding it.
	DWORD timeout = 100ul;
	if ( 0 != setsockopt(SocketToServer->handle(), SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, (int)sizeof(DWORD)) )
	{
		std::string msg( "setsockopt error: " );
		gService( )->console( ).printFailure( FailureLevel::WARNING, msg+std::to_string(WSAGetLastError()) );
	}
	if ( char queueServerIPAddress[ ] = "192.168.219.102";
		 -1 == SocketToServer->connect(EndPoint(queueServerIPAddress, QUEUE_SERVER_PORT)) )
	{
		// Exception
		gService( )->console( ).printFailure( FailureLevel::WARNING, "Failed to connect to the queue server.\n" );
		// Triggering
		++mFrameCount_disconnection;
	}
	else
	{
		gService( )->console( ).print( "Succeeded to connect to the queue server.", sf::Color::Green );
		const Dword version = gService( )->vault( ).at( HK_VERSION );
//TODO: OTP로 신원 확인
		ReceivingResult = -2;
		ThreadToReceive = std::make_unique< std::thread >( &Receive, std::ref(*SocketToServer) );
	}
	setScene( ::scene::online::ID::WAITING );
	loadResources( );

	IsInstantiated = true;
}

::scene::online::Online::~Online( )
{
	IsReceiving = false;
	std::this_thread::sleep_for( std::chrono::milliseconds(150) );
	CvForResumingRcv.notify_all( );
	if ( nullptr != ThreadToReceive && true == ThreadToReceive->joinable() )
	{
		ThreadToReceive->join( );
	}
	ThreadToReceive.reset( );
	FrameCount_interval = 0;
	SocketToServer->close( );
	SocketToServer.reset( );

	IsInstantiated = false;
}

void ::scene::online::Online::loadResources( )
{
	mCurrentScene->loadResources( );

	bool isPathDefault = true;
	bool isWDefault = true;
	bool isHDefault = true;
	lua_State* lua = luaL_newstate( );
	char scriptPathNName[] = "Scripts/Online.lua";
	if ( true == luaL_dofile(lua, scriptPathNName) )
	{
		// File Not Found Exception
		gService( )->console( ).printFailure( FailureLevel::FATAL, std::string("File Not Found: ")+scriptPathNName );
		lua_close( lua );
	}
	else
	{
		luaL_openlibs( lua );
		const int TOP_IDX = -1;
		const std::string tableName0( "Sprite" );
		lua_getglobal( lua, tableName0.data() );
		// Type Check Exception
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK, tableName0.data(), scriptPathNName );
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
					gService( )->console( ).printScriptError( ExceptionType::FILE_NOT_FOUND,
						(tableName0+":"+field0).data(), scriptPathNName );
				}
				else
				{
					isPathDefault = false;
				}
			}
			// Type Check Exception
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName0+":"+field0).data(), scriptPathNName );
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
					gService( )->console( ).printScriptError( ExceptionType::RANGE_CHECK,
						(tableName0+":"+field1).data(), scriptPathNName );
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
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName0+":"+field1).data(), scriptPathNName );
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
					gService( )->console( ).printScriptError( ExceptionType::RANGE_CHECK,
						(tableName0+":"+field2).data(), scriptPathNName );
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
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName0+":"+field2).data(), scriptPathNName );
			}
			lua_pop( lua, 2 );
		}
		lua_close( lua );
	}

	if ( true == isPathDefault )
	{
		const char defaultFilePathNName[ ] = "Images/Online.png";
		if ( false == mTexture.loadFromFile(defaultFilePathNName) )
		{
			// Exception: When there's not even the default file,
			gService( )->console( ).printFailure( FailureLevel::FATAL, std::string( "File Not Found: " )+defaultFilePathNName );
#ifdef _DEBUG
			__debugbreak( );
#endif
		}
	}

	if ( true == isWDefault || true == isHDefault )
	{
		gService( )->console( ).print( "Default: width 256, height 128" );
	}

	mSprite.setTexture( mTexture );
	const sf::Vector2i cast( mSpriteClipSize_ );
	mSprite.setTextureRect( sf::IntRect(0, cast.y, cast.x, cast.y) );
	mSprite.setPosition( (sf::Vector2f(mWindow_.getSize())-mSpriteClipSize_)*0.5f );
}

::scene::ID scene::online::Online::update( std::list<sf::Event>& eventQueue )
{
	::scene::ID retVal = ::scene::ID::AS_IS;
	if ( 0u == mFrameCount_disconnection )
	{
		const ::scene::online::ID nextSceneID = mCurrentScene->update( eventQueue );
		if ( ::scene::online::ID::AS_IS < nextSceneID )
		{
			setScene( nextSceneID );
		}
		else if ( ::scene::online::ID::MAIN_MENU == nextSceneID )
		{
			retVal = ::scene::ID::MAIN_MENU;
		}
	}
	else if ( SECONDS_TO_MAIN_MENU*mFPS_ < mFrameCount_disconnection )
	{
		retVal = ::scene::ID::MAIN_MENU;
	}
	return retVal;
}

void ::scene::online::Online::draw( )
{
	if ( 0u == mFrameCount_disconnection )
	{
		mCurrentScene->draw( );
	}
	else
	{
		mWindow_.draw( mSprite );
		++mFrameCount_disconnection;
	}
	++FrameCount_interval;
}

bool scene::online::Online::connectToMainServer( )
{
	SocketToServer = std::make_unique< Socket >( Socket::Type::TCP );
	ASSERT_TRUE( -1 != SocketToServer->bind(EndPoint::Any) );
	// NOTE: Socket option should be set following binding it.
	DWORD timeout = 100ul;
	if ( 0 != setsockopt(SocketToServer->handle(), SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, (int)sizeof(DWORD)) )
	{
		std::string msg( "setsockopt error: " );
		gService( )->console( ).printFailure( FailureLevel::WARNING, msg+std::to_string(WSAGetLastError()) );
		return false;
	}
	if ( char mainServerIPAddress[ ] = "192.168.219.102";
		-1 == SocketToServer->connect(EndPoint(mainServerIPAddress, MAIN_SERVER_PORT)) )
	{
		// Exception
		gService( )->console( ).printFailure( FailureLevel::WARNING, "Failed to connect to the main server.\n" );
		// Triggering
		++mFrameCount_disconnection;
		return false;
	}
	gService( )->console( ).print( "Succeeded to connect to the main server.", sf::Color::Green );
	return true;
}

void scene::online::Online::disconnect( )
{
	// Triggering
	++mFrameCount_disconnection;
}

void scene::online::Online::send( char* const data, const int size )
{
	if( -1 == SocketToServer->sendOverlapped(data, size) )
	{
		disconnect( );
	}
}

void scene::online::Online::send( Packet& packet )
{
	std::string& data =	packet.data();
	send( data.data(), (int)data.size() );
}

void scene::online::Online::sendZeroByte()
{
	char ignored = ' ';
	send( &ignored, 0 );
}

void scene::online::Online::receive( ) const
{
	if ( 0 == ReceivingResult ||
		-1 == ReceivingResult && WSAETIMEDOUT != Error )
	{
		gService()->console().printFailure( FailureLevel::WARNING, "Can't receive." );
		return;
	}

	ReceivingResult = -2;
	Error = 0;
	if ( nullptr == ThreadToReceive )
	{
		ThreadToReceive = std::make_unique< std::thread >( &Receive, std::ref(*SocketToServer) );
	}
	else
	{
		CvForResumingRcv.notify_one( );
	}
}

bool scene::online::Online::hasReceived( const uint32_t intervalMs )
{
	uint32_t converted = 0;
	if ( 16 < intervalMs )
	{
		converted = intervalMs*mFPS_/1000u;
	}

	if ( converted <= FrameCount_interval && 0 < ReceivingResult )
	{
		FrameCount_interval = 0u;
		return true;
	}
	else
	{
		if ( 0 == ReceivingResult ||
			-1 == ReceivingResult && WSAETIMEDOUT != Error )
		{
			gService( )->console( ).printFailure( FailureLevel::FATAL, "Failed to receive." );
			// Triggering
			++mFrameCount_disconnection;
		}
		return false;
	}
}

std::optional<std::string> scene::online::Online::getByTag( const Tag tag,
														   const Online::Option option,
														   const uint32_t size ) const
{
	ASSERT_TRUE( 0 < ReceivingResult );
	// TODO: 컴파일 타임에 판단하고 싶다.
	ASSERT_FALSE( Online::Option::INDETERMINATE_SIZE == option && 0 != size );
	ASSERT_FALSE( Online::Option::SPECIFIED_SIZE == option && 0 == size );

	const char* const rcvBuf = SocketToServer->receivingBuffer( );
	std::string_view strView( rcvBuf, ReceivingResult );
	uint32_t beginPos = -1;
	if ( option & Option::FIND_END_TO_BEGIN )
	{
		uint32_t off = 0;
		while ( true )
		{
			size_t pos = strView.find( tag, off );
			if ( std::string_view::npos != pos )
			{
				beginPos = (uint32_t)pos;
				off = (uint32_t)++pos;
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		if ( const size_t pos = strView.find(tag);
			std::string_view::npos != pos )
		{
			beginPos = (uint32_t)pos;
		}
	}

	if ( -1 == beginPos )
	{
		return std::nullopt;
	}

	uint32_t endPos = -1;
	uint32_t dataPos = beginPos + (uint32_t)std::strlen(tag);
	if ( option & Option::INDETERMINATE_SIZE )
	{
		const uint32_t _size = ::ntohl(*(uint32_t*)&rcvBuf[dataPos]);
		dataPos += (sizeof(_size) + 1);
		endPos = (uint32_t)dataPos + _size;
	}
	else
	{
		endPos = dataPos + size;
	}

	ASSERT_TRUE( -1 != endPos );

	if ( !(option & Option::RETURN_TAG_ATTACHED) )
	{
		beginPos = dataPos;
	}

	return std::string( &strView[beginPos], endPos-beginPos );
}

void scene::online::Online::stopReceivingFromQueueServer( ) const
{
	IsReceiving = false;
	CvForResumingRcv.notify_one( );
	if ( true == ThreadToReceive->joinable() )
	{
		ThreadToReceive->join( );
	}
	ThreadToReceive.reset( );
}

#ifdef _DEV
::scene::ID scene::online::Online::currentScene( ) const
{
	return ::scene::ID::ONLINE_BATTLE;
}
#endif

void scene::online::Online::setScene( const::scene::online::ID nextSceneID )
{
	mCurrentScene.reset( );
	switch ( nextSceneID )
	{
		case ::scene::online::ID::WAITING:
			mCurrentScene = std::make_unique< ::scene::online::Waiting >( mWindow_, *this );
			break;
		case ::scene::online::ID::IN_LOBBY:
			mCurrentScene = std::make_unique< ::scene::online::InLobby >( mWindow_, *this );
			break;
		case ::scene::online::ID::IN_ROOM_AS_HOST:
			mCurrentScene = std::make_unique< ::scene::online::InRoom >( mWindow_, *this, true );
			break;
		case ::scene::online::ID::IN_ROOM:
			mCurrentScene = std::make_unique< ::scene::online::InRoom >( mWindow_, *this );
			break;
		default:
#ifdef _DEBUG
			__debugbreak( );
#else
			__assume( 0 );
#endif
	}
}