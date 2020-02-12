#include "../../pch.h"
#include "Online.h"
#include <Lib/Common.h>
#include "../../ServiceLocatorMirror.h"
#include "../VaultKeyList.h"
#include "Waiting.h"
#include "InLobby.h"
#include "InRoom.h"

bool ::scene::online::Online::IsInstantiated = false;

namespace
{
	const uint8_t SECONDS_TO_MAIN_MENU = 3;
	const uint32_t RECEIVING_WAIT_MS = 100;
	bool IsReceiving;
	int ReceivingResult;
	int ReceivingError;
	std::unique_ptr< Socket > SocketToServer;
	std::unique_ptr< std::thread > ThreadToReceive;
	std::condition_variable CvForResumingRcv;
	std::mutex ReceivingMutex;

	void Receive( Socket& socket )
	{
		IsReceiving = true;
		while ( true == IsReceiving )
		{
			ReceivingResult = socket.receiveBlocking();
			ReceivingError = WSAGetLastError();
			if ( -1 == ReceivingResult && WSAETIMEDOUT == ReceivingError )
			{
				// Trying to receive again
			}
			else if ( 0 < ReceivingResult )
			{
				std::unique_lock uLock( ReceivingMutex );
				CvForResumingRcv.wait( uLock );
			}
			else
			{
 				break;
			}
		}
	}

	void CompletionRoutine(DWORD, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD)
	{
		SocketToServer->completedIO( lpOverlapped, cbTransferred );
	}
}

scene::online::Online::Online( sf::RenderWindow& window )
	: mFrameCount_disconnection( 0 ), mMyNicknameHashed_( 0 ),
	mWindow_( window )
{
	ASSERT_FALSE( IsInstantiated );

	ReceivingResult = -2;
	mFPS_ = (uint32_t)gService()->vault()[HK_FORE_FPS];
	setScene( ::scene::online::ID::WAITING );
	loadResources( );

	IsInstantiated = true;
}

::scene::online::Online::~Online( )
{
	IsReceiving = false;
	std::this_thread::sleep_for( std::chrono::milliseconds(RECEIVING_WAIT_MS+50) );
	CvForResumingRcv.notify_all( );
	if ( nullptr != ThreadToReceive && true == ThreadToReceive->joinable() )
	{
		ThreadToReceive->join( );
	}
	ThreadToReceive.reset( );
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
		const std::string tableName0( "DisconnectionSprite" );
		lua_getglobal( lua, tableName0.data() );
		// Type Check Exception
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK, tableName0, scriptPathNName );
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
						tableName0+':'+field0, scriptPathNName );
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
					tableName0+':'+field0, scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field1[ ] = "clipWidth";
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
						tableName0+':'+field1, scriptPathNName );
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
					tableName0+':'+field1, scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field2[ ] = "clipHeight";
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
						tableName0+':'+field2, scriptPathNName );
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
					tableName0+':'+field2, scriptPathNName );
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

::scene::ID scene::online::Online::update( std::vector<sf::Event>& eventQueue )
{
	::scene::ID retVal = ::scene::ID::AS_IS;

	if ( 0 == mFrameCount_disconnection )
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
	if ( 0 == mFrameCount_disconnection )
	{
		mCurrentScene->draw( );
	}
	else
	{
		mWindow_.draw( mSprite );
		++mFrameCount_disconnection;
	}
}

void scene::online::Online::connectToQueueServer()
{
	SocketToServer = std::make_unique<Socket>(Socket::Type::TCP);
	if ( -1 == SocketToServer->bind(EndPoint::Any) )
	{
		// Exception
		gService( )->console( ).printFailure( FailureLevel::WARNING,
											 "Failed to bind a to-queue-server socket.\n" );
		// Triggering
		mFrameCount_disconnection = 1;
		return;
	}
	// NOTE: Socket option should be set after binding it.
	if ( const DWORD timeout = RECEIVING_WAIT_MS;
			 -1 == setsockopt(SocketToServer->handle(), SOL_SOCKET, SO_RCVTIMEO,
		(char*)&timeout, sizeof(DWORD)) )
	{
		// Exception
		std::string msg( "setsockopt error: " );
		gService( )->console( ).printFailure( FailureLevel::WARNING,
											 msg+std::to_string(WSAGetLastError()) );
		// Triggering
		mFrameCount_disconnection = 1;
		return;
	}
	if ( -1 == SocketToServer->connect(EndPoint(QUEUE_SERVER_IP_ADDRESS, QUEUE_SERVER_PORT)) )
	{
		// Exception
		gService( )->console( ).printFailure( FailureLevel::WARNING,
											 "Failed to connect to the queue server.\n" );
		// Triggering
		mFrameCount_disconnection = 1;
		return;
	}
	gService( )->console( ).print( "Succeeded to connect to the queue server.",
								  sf::Color::Green );
}

void scene::online::Online::disconnect( )
{
	// Triggering
	mFrameCount_disconnection = 1;
}

bool scene::online::Online::connectToMainServer( )
{
	bool result = true;
	IsReceiving = false;
	std::this_thread::sleep_for( std::chrono::milliseconds(RECEIVING_WAIT_MS+50) );
	CvForResumingRcv.notify_one( );
	if ( true == ThreadToReceive->joinable() )
	{
		ThreadToReceive->join( );
	}
	ThreadToReceive.reset( );
	SocketToServer->close( );
	SocketToServer.reset( );
	SocketToServer = std::make_unique<Socket>(Socket::Type::TCP);
	if ( -1 == SocketToServer->bind(EndPoint::Any) )
	{
		// Exception
		gService( )->console( ).printFailure( FailureLevel::WARNING, "Failed to bind a to-main-server socket.\n" );
		// Triggering
		mFrameCount_disconnection = 1;
		result = false;
		return result;
	}
	// NOTE: Socket option should be set after binding it.
	if ( const DWORD timeout = RECEIVING_WAIT_MS;
		-1 == setsockopt(SocketToServer->handle(), SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, (int)sizeof(DWORD)) )
	{
		// Exception
		std::string msg( "setsockopt error: " );
		gService( )->console( ).printFailure( FailureLevel::WARNING, msg+std::to_string(WSAGetLastError()) );
		// Triggering
		mFrameCount_disconnection = 1;
		result = false;
		return result;
	}
	if ( const DWORD SND_BUF_SIZ = 0;
		-1 == setsockopt(SocketToServer->handle(), SOL_SOCKET, SO_SNDBUF, (char*)&SND_BUF_SIZ, (int)sizeof(DWORD)) )
	{
		// Exception
		std::string msg( "setsockopt error: " );
		gService( )->console( ).printFailure( FailureLevel::WARNING, msg+std::to_string(WSAGetLastError()) );
		// Triggering
		mFrameCount_disconnection = 1;
		result = false;
		return result;
	}
	if ( -1 == SocketToServer->connect(EndPoint(MAIN_SERVER_IP_ADDRESS, MAIN_SERVER_PORT)) )
	{
		// Exception
		gService( )->console( ).printFailure( FailureLevel::WARNING, "Failed to connect to the main server.\n" );
		// Triggering
		mFrameCount_disconnection = 1;
		result = false;
		return result;
	}
	gService( )->console( ).print( "Succeeded to connect to the main server.", sf::Color::Green );
	return true;
}

void scene::online::Online::send( char* const data, const int size )
{
	if ( -1 == SocketToServer->sendOverlapped(data, size, CompletionRoutine) )
	{
		disconnect( );
		return;
	}
	// NOTE: Blocking can happen.
	::SleepEx( 1000, TRUE );
}

void scene::online::Online::send( Packet& packet )
{
	if ( -1 == SocketToServer->sendOverlapped(packet, CompletionRoutine) )
	{
		disconnect( );
		return;
	}
	// NOTE: Blocking can happen.
	::SleepEx( 1000, TRUE );
}

void scene::online::Online::receive( )
{
	if ( 0 == ReceivingResult ||
		-1 == ReceivingResult && WSAETIMEDOUT != ReceivingError )
	{
		gService()->console().printFailure( FailureLevel::WARNING, "Can't receive." );
		// Triggering
		mFrameCount_disconnection = 1;
		return;
	}

	ReceivingResult = -2;
	ReceivingError = 0;
	if ( nullptr == ThreadToReceive )
	{
		ThreadToReceive = std::make_unique<std::thread>(&Receive, std::ref(*SocketToServer));
	}
	else
	{
		CvForResumingRcv.notify_one( );
	}
}

bool scene::online::Online::hasReceived( )
{
	if ( 0 < ReceivingResult )
	{
		return true;
	}
	else
	{
		if ( 0 == ReceivingResult ||
			-1 == ReceivingResult && WSAETIMEDOUT != ReceivingError )
		{
			gService( )->console( ).printFailure( FailureLevel::FATAL, "Failed to receive." );
			// Triggering
			mFrameCount_disconnection = 1;
		}
		return false;
	}
}

std::optional<std::string> scene::online::Online::getByTag( const Tag tag,
														   const Online::Option option,
														   uint32_t bodySize ) const
{
	ASSERT_FALSE( 0 == bodySize && Online::Option::RETURN_TAG_ATTACHED != option );
	ASSERT_TRUE( 0 < ReceivingResult );

	std::string _retVal;
	std::string& extraRcvBuf = SocketToServer->extraReceivingBuffer();
	if ( false == extraRcvBuf.empty() &&
		0 == extraRcvBuf.find(tag) )
	{
		if ( option & Online::Option::RETURN_TAG_ATTACHED )
		{
			_retVal.append( extraRcvBuf, 0, extraRcvBuf.size() );
			
		}
		else
		{
			const uint32_t tagLen =	(uint32_t)std::strlen(tag);
			_retVal.append( extraRcvBuf, tagLen, extraRcvBuf.size()-tagLen );
		}
		return _retVal;
	}

	const char* const rcvBuf = SocketToServer->receivingBuffer();
	std::string_view strView( rcvBuf, ReceivingResult );
	uint32_t beginPos = -1;
	if ( option & Option::FIND_END_TO_BEGIN )
	{
		uint32_t off = 0;
		while ( true )
		{
			size_t pos = strView.find(tag, off);
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
	
	const uint32_t tagLen = (uint32_t)std::strlen(tag);
	if ( option & Option::RETURN_TAG_ATTACHED )
	{
		_retVal.append( &rcvBuf[beginPos], tagLen );
		if ( 0 == bodySize )
		{
			return _retVal;
		}
	}
	uint32_t& curPos = beginPos;
	curPos += tagLen;
	if ( -1 == bodySize )
	{
		const uint32_t sight = Socket::RCV_BUF_SIZ - curPos;
		if ( sizeof(uint32_t) <= sight )
		{
			bodySize = ::ntohl(*(uint32_t*)&rcvBuf[curPos]);
		}
		else
		{
			char _bodySize[sizeof(uint32_t)] = { '\0' };
			if ( 0 != sight )
			{
				::memcpy_s( _bodySize, sizeof(uint32_t), &rcvBuf[curPos], sight );
			}
			if ( -1 == ::recv(SocketToServer->handle(), &_bodySize[sight], sizeof(uint32_t)-sight, 0) )
			{
				__debugbreak();
			}
			bodySize = ::ntohl(*(uint32_t*)_bodySize);
		}
		curPos += sizeof(uint32_t);
	}
	const uint32_t sight = Socket::RCV_BUF_SIZ - curPos;
	if ( bodySize <= sight )
	{
		_retVal.append( &rcvBuf[curPos], bodySize );
	}
	else
	{
		extraRcvBuf.resize( tagLen + bodySize );
		extraRcvBuf += _retVal; 
		char* _extraRcvBuf = extraRcvBuf.data();
		_extraRcvBuf += tagLen;
		if ( -1 == ::recv(SocketToServer->handle(), _extraRcvBuf, bodySize, 0) )
		{
			__debugbreak();
		}
		_retVal.append( _extraRcvBuf, bodySize );
	}

	return _retVal;
}

void scene::online::Online::setMyNickname( std::string& myNickname )
{
	mMyNickname = myNickname;
	mMyNicknameHashed_ = ::util::hash::Digest( myNickname.data(), (uint8_t)myNickname.size() );
}

const std::string& scene::online::Online::myNickname( ) const
{
	return mMyNickname;
}

HashedKey scene::online::Online::myNicknameHashed( ) const
{
	return mMyNicknameHashed_;
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