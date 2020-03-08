#include "../../pch.h"
#include "Online.h"
#include "../../ServiceLocatorMirror.h"
#include "Waiting.h"
#include "InLobby.h"
#include "InRoom.h"

bool ::scene::online::Online::IsInstantiated = false;

namespace
{
	const uint8_t SECONDS_TO_MAIN_MENU = 3;
	const uint16_t RECEIVING_WAIT_MS = 100;
	bool IsReceiving;
	int ReceivingResult;
	int ReceivingError;
	std::unique_ptr<Socket> SocketToServer;
	std::unique_ptr<std::thread> ThreadToReceive;
	std::condition_variable CvForResumingRcv;
	std::mutex MutexForConditionVariable, MutexForReceivingResult;

	void Receive( Socket& socket )
	{
		IsReceiving = true;
		while ( true == IsReceiving )
		{
			int rcvRes = socket.receiveBlocking();
			int rcvErr = WSAGetLastError();
			if ( -1 == rcvRes && WSAETIMEDOUT == rcvErr )
			{
				// Trying to receive again.
			}
			else if ( 0 < rcvRes )
			{
				{
					std::scoped_lock lock( MutexForReceivingResult );
					ReceivingResult = rcvRes;
				}
				std::unique_lock uLock( MutexForConditionVariable );
				CvForResumingRcv.wait( uLock );
			}
			else
			{
				{
					std::scoped_lock lock( MutexForReceivingResult );
					ReceivingResult = rcvRes;
					ReceivingError = rcvErr;
				}
  				break;
			}
		}
	}

	void CompletionRoutine( DWORD, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD )
	{
		SocketToServer->completedIO( lpOverlapped, cbTransferred );
	}
}

scene::online::Online::Online( const sf::RenderWindow& window )
	: mFrameCountToMainMenu( 0 ), mMyNicknameHashed_( 0 ), mWindow( window )
{
	ASSERT_TRUE( false == IsInstantiated );

	ReceivingResult = -2;
	ReceivingError = 0;
	auto& vault = gService()->vault();
	const auto it = vault.find(HK_FORE_FPS);
	ASSERT_TRUE( vault.end() != it );
	mFPS_ = (uint16_t)it->second;
	setScene( ::scene::online::ID::WAITING );
	loadResources( window );

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

void ::scene::online::Online::loadResources( const sf::RenderWindow& window )
{
	mCurrentScene->loadResources( window );

	bool isPathDefault = true;
	bool isWDefault = true;
	bool isHDefault = true;
	lua_State* lua = luaL_newstate();
	const std::string scriptPath( "Scripts/Online.lua" );
	if ( true == luaL_dofile(lua, scriptPath.data()) )
	{
		gService()->console().printFailure( FailureLevel::FATAL,
										   "File Not Found: "+scriptPath );
		lua_close( lua );
	}
	else
	{
		luaL_openlibs( lua );
		const int TOP_IDX = -1;
		std::string tableName( "DisconnectionSprite" );
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK, tableName, scriptPath );
		}
		else
		{
			std::string field( "path" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			int type = lua_type(lua, TOP_IDX);
			if ( LUA_TSTRING == type )
			{
				if ( false == mTexture.loadFromFile(lua_tostring(lua, TOP_IDX)) )
				{
					gService()->console().printScriptError( ExceptionType::FILE_NOT_FOUND,
						tableName+':'+field, scriptPath );
				}
				else
				{
					isPathDefault = false;
				}
			}
			else
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
					tableName+':'+field, scriptPath );
			}
			lua_pop( lua, 1 );

			field = "clipWidth";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				const float temp = (float)lua_tonumber(lua, TOP_IDX);
				// Range Check Exception
				if ( 0 > temp )
				{
					gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
						tableName+':'+field, scriptPath );
				}
				// When the value looks OK,
				else
				{
					mSpriteClipSize.x = temp;
					isWDefault = false;
				}
			}
			else if ( LUA_TNIL != type )
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
					tableName+':'+field, scriptPath );
			}
			lua_pop( lua, 1 );

			field = "clipHeight";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				const float temp = (float)lua_tonumber(lua, TOP_IDX);
				// Range Check Exception
				if ( 0 > temp )
				{
					gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
						tableName+':'+field, scriptPath );
				}
				// When the value looks OK,
				else
				{
					mSpriteClipSize.y = temp;
					isHDefault = false;
				}
			}
			else if ( LUA_TNIL != type )
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
					tableName+':'+field, scriptPath );
			}
			lua_pop( lua, 2 );
		}
		lua_close( lua );
	}

	if ( true == isPathDefault )
	{
		std::string defaultFilePath( "Images/Online.png" );
		if ( false == mTexture.loadFromFile(defaultFilePath) )
		{
			gService()->console().printFailure( FailureLevel::WARNING,
											   "File Not Found: "+defaultFilePath );
		}
	}

	if ( true == isWDefault || true == isHDefault )
	{
		gService()->console().print( "Default: width 256, height 128" );
	}

	mSprite.setTexture( mTexture );
	const sf::Vector2i cast( mSpriteClipSize );
	mSprite.setTextureRect( sf::IntRect(0, cast.y, cast.x, cast.y) );
	mSprite.setPosition( (sf::Vector2f(window.getSize())-mSpriteClipSize)*0.5f );
}

::scene::ID scene::online::Online::update( std::vector<sf::Event>& eventQueue )
{
	::scene::ID retVal = ::scene::ID::AS_IS;

	if ( 0 == mFrameCountToMainMenu )
	{
		const ::scene::online::ID nextSceneID = mCurrentScene->update(eventQueue, *this, mWindow);
		if ( ::scene::online::ID::AS_IS < nextSceneID )
		{
			setScene( nextSceneID );
		}
		else if ( ::scene::online::ID::MAIN_MENU == nextSceneID )
		{
			retVal = ::scene::ID::MAIN_MENU;
		}
	}
	else if ( SECONDS_TO_MAIN_MENU*mFPS_ < mFrameCountToMainMenu )
	{
		retVal = ::scene::ID::MAIN_MENU;
	}
	return retVal;
}

void ::scene::online::Online::draw( sf::RenderWindow& window )
{
	if ( 0 == mFrameCountToMainMenu )
	{
		mCurrentScene->draw( window );
	}
	else
	{
		window.draw( mSprite );
		++mFrameCountToMainMenu;
	}
}

void scene::online::Online::connectToQueueServer( )
{
	SocketToServer = std::make_unique<Socket>(Socket::Type::TCP);
	if ( -1 == SocketToServer->bind(EndPoint::Any) )
	{
		// Exception
		gService()->console().printFailure( FailureLevel::WARNING,
											 "Failed to bind a to-queue-server socket.\n" );
		// Triggering
		mFrameCountToMainMenu = 1;
		return;
	}
	// NOTE: Socket option should be set after binding it.
	if ( const DWORD timeout = RECEIVING_WAIT_MS;
			 -1 == setsockopt(SocketToServer->handle(), SOL_SOCKET, SO_RCVTIMEO,
		(char*)&timeout, sizeof(DWORD)) )
	{
		// Exception
		std::string msg( "setsockopt error: " );
		gService()->console().printFailure( FailureLevel::WARNING,
											 msg+std::to_string(WSAGetLastError()) );
		// Triggering
		mFrameCountToMainMenu = 1;
		return;
	}
	if ( -1 == SocketToServer->connect(EndPoint(QUEUE_SERVER_IP_ADDRESS, QUEUE_SERVER_PORT)) )
	{
		// Exception
		gService()->console().printFailure( FailureLevel::WARNING,
											 "Failed to connect to the queue server.\n" );
		// Triggering
		mFrameCountToMainMenu = 1;
		return;
	}
	gService()->console().print( "Succeeded to connect to the queue server.",
								  sf::Color::Green );
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
		gService()->console().printFailure( FailureLevel::WARNING, "Failed to bind a to-main-server socket.\n" );
		// Triggering
		mFrameCountToMainMenu = 1;
		result = false;
		return result;
	}
	// NOTE: Socket option should be set only after binding it.
	if ( const DWORD timeout = RECEIVING_WAIT_MS;
		-1 == setsockopt(SocketToServer->handle(), SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, (int)sizeof(DWORD)) )
	{
		// Exception
		std::string msg( "setsockopt error: " );
		gService()->console().printFailure( FailureLevel::WARNING, msg+std::to_string(WSAGetLastError()) );
		// Triggering
		mFrameCountToMainMenu = 1;
		result = false;
		return result;
	}
	if ( const DWORD SND_BUF_SIZ = 0;
		-1 == setsockopt(SocketToServer->handle(), SOL_SOCKET, SO_SNDBUF, (char*)&SND_BUF_SIZ, (int)sizeof(DWORD)) )
	{
		// Exception
		std::string msg( "setsockopt error: " );
		gService()->console().printFailure( FailureLevel::WARNING, msg+std::to_string(WSAGetLastError()) );
		// Triggering
		mFrameCountToMainMenu = 1;
		result = false;
		return result;
	}
	if ( -1 == SocketToServer->connect(EndPoint(MAIN_SERVER_IP_ADDRESS, MAIN_SERVER_PORT)) )
	{
		// Exception
		gService()->console().printFailure( FailureLevel::WARNING, "Failed to connect to the main server.\n" );
		// Triggering
		mFrameCountToMainMenu = 1;
		result = false;
		return result;
	}
	gService()->console().print( "Succeeded to connect to the main server.", sf::Color::Green );
	return true;
}

void scene::online::Online::disconnect( )
{
	// Triggering
	mFrameCountToMainMenu = 1;
}

void scene::online::Online::send( char* const data, const int size )
{
	if ( -1 == SocketToServer->sendOverlapped(data, size, CompletionRoutine) )
	{
		disconnect( );
		return;
	}
	// Invocation of CompletionRoutine requires this.  Blocking can happen.
	::SleepEx( 1000, TRUE );
}

void scene::online::Online::send( Packet& packet )
{
	if ( -1 == SocketToServer->sendOverlapped(packet, CompletionRoutine) )
	{
		disconnect( );
		return;
	}
	// Invocation of CompletionRoutine requires this.  Blocking can happen.
	::SleepEx( 1000, TRUE );
}

void scene::online::Online::receive( )
{
	{
		std::scoped_lock lock( MutexForReceivingResult );
		ReceivingResult = -2;
		ReceivingError = 0;
	}

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
	int rcvRes, rcvErr;
	{
		std::scoped_lock lock( MutexForReceivingResult );
		rcvRes = ReceivingResult;
		rcvErr = ReceivingError;
	}
	if ( 0 < rcvRes )
	{
		return true;
	}
	else
	{
		if ( 0 == rcvRes )
		{
			gService()->console().printFailure( FailureLevel::WARNING, "Server disconnected." );
			disconnect( );
		}
		else if ( -1 == rcvRes && WSAETIMEDOUT != rcvErr )
		{
			gService()->console().printFailure( FailureLevel::WARNING, "Failed to receive." );
			disconnect( );
		}
		return false;
	}
}

std::optional<std::string> scene::online::Online::getByTag( const Tag tag,
														   const Online::Option option,
														   uint16_t bodySize ) const
{
	ASSERT_FALSE( 0 == bodySize && Online::Option::RETURNING_TAG_ATTACHED != option );
	ASSERT_TRUE( 0 < ReceivingResult );

	std::string _retVal;
	std::string& extraRcvBuf = SocketToServer->extraReceivingBuffer();
	if ( false == extraRcvBuf.empty() &&
		0 == extraRcvBuf.find(tag) )
	{
		if ( option & Online::Option::RETURNING_TAG_ATTACHED )
		{
			_retVal.append( extraRcvBuf, 0, extraRcvBuf.size() );
			
		}
		else
		{
			const uint8_t tagLen =	(uint8_t)std::strlen(tag);
			_retVal.append( extraRcvBuf, tagLen, extraRcvBuf.size()-tagLen );
		}
		return _retVal;
	}

	const char* const rcvBuf = SocketToServer->receivingBuffer();
	std::string_view strView( rcvBuf, ReceivingResult );
	uint16_t beginPos = -1;
	if ( option & Option::FINDING_END_TO_BEGIN )
	{
		uint16_t off = 0;
		while ( true )
		{
			size_t pos = strView.find(tag, off);
			if ( std::string_view::npos != pos )
			{
				beginPos = (uint16_t)pos;
				off = (uint16_t)++pos;
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
			beginPos = (uint16_t)pos;
		}
	}
	if ( UINT16_MAX == beginPos )
	{
		return std::nullopt;
	}
	
	const uint8_t tagLen = (uint8_t)std::strlen(tag);
	if ( option & Option::RETURNING_TAG_ATTACHED )
	{
		_retVal.append( &rcvBuf[beginPos], tagLen );
		if ( 0 == bodySize )
		{
			return _retVal;
		}
	}
	uint16_t& curPos = beginPos;
	curPos += tagLen;
	if ( UINT16_MAX == bodySize )
	{
		const uint16_t sight = Socket::RCV_BUF_SIZ - curPos;
		if ( sizeof(uint16_t) <= sight )
		{
			bodySize = ::ntohs(*(uint16_t*)&rcvBuf[curPos]);
		}
		else
		{
			char _bodySize[sizeof(uint16_t)] = { '\0' };
			if ( 0 != sight )
			{
				::memcpy_s( _bodySize, sizeof(uint16_t), &rcvBuf[curPos], sight );
			}
			// Fetching a part from receiving buffer on O/S level.
			ASSERT_TRUE( -1 != ::recv(SocketToServer->handle(), &_bodySize[sight], sizeof(uint16_t)-sight, 0) );
			bodySize = ::ntohs(*(uint16_t*)_bodySize);
		}
		curPos += sizeof(uint16_t);
	}
	const uint16_t sight = Socket::RCV_BUF_SIZ - curPos;
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
		// Fetching a part from receiving buffer on O/S level.
		ASSERT_TRUE( -1 != ::recv(SocketToServer->handle(), _extraRcvBuf, bodySize, 0) );
		_retVal.append( _extraRcvBuf, bodySize );
	}

	return _retVal;
}

void scene::online::Online::setMyNickname( std::string& myNickname )
{
	mMyNickname = myNickname;
	mMyNicknameHashed_ = ::util::hash::Digest2(myNickname);
}

const std::string& scene::online::Online::myNickname( ) const
{
	return mMyNickname;
}

HashedKey scene::online::Online::myNicknameHashed( ) const
{
	return mMyNicknameHashed_;
}

::scene::ID scene::online::Online::currentScene( ) const
{
	return ::scene::ID::ONLINE_BATTLE;
}

void scene::online::Online::setScene( const::scene::online::ID nextSceneID )
{
	mCurrentScene.reset( );
	switch ( nextSceneID )
	{
		case ::scene::online::ID::WAITING:
			mCurrentScene = std::make_unique<::scene::online::Waiting>( mWindow, *this );
			break;
		case ::scene::online::ID::IN_LOBBY:
			mCurrentScene = std::make_unique<::scene::online::InLobby>( mWindow, *this );
			break;
		case ::scene::online::ID::IN_ROOM_AS_HOST:
			mCurrentScene = std::make_unique<::scene::online::InRoom>( mWindow, *this, true );
			break;
		case ::scene::online::ID::IN_ROOM:
			mCurrentScene = std::make_unique<::scene::online::InRoom>( mWindow, *this );
			break;
		default:
#ifdef _DEBUG
			__debugbreak( );
#else
			__assume( 0 );
#endif
	}
}