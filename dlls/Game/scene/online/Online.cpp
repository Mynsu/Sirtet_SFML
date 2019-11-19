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
	std::unique_ptr< std::thread > Thread0, Thread1;
	bool IsSending, IsReceiving;
	int8_t SendingResult, ReceivingResult;
	std::condition_variable CvForResumingSnd, CvForResumingRcv;
	std::mutex MutexSnd, MutexRcvBuf;
	char* DataToSend = nullptr;
	// Unit: Byte
	int DataSizeToSend = -1;
	const uint8_t SND_BUF_SIZ = 16;
	char SendingBuffer[ SND_BUF_SIZ ] = {0,};
	
	uint32_t FrameCount_interval = 0;

	void Send( Socket& socket, const uint32_t initialDelayMs = 1000u )
	{
		// Reset
		SendingResult = -2;
		DataToSend = SendingBuffer;
		std::this_thread::sleep_for( std::chrono::milliseconds(initialDelayMs) );
		IsSending = true;
		while ( true == IsSending )
		{
			SendingResult = socket.sendBlock( DataToSend, DataSizeToSend );
			{
				std::unique_lock uLock( MutexSnd );
				CvForResumingSnd.wait( uLock );
			}
			SendingResult = -2;
		}
	}

	void Receive( Socket& socket )
	{
		// Reset
		ReceivingResult = -2;
		IsReceiving = true;

		while ( true == IsReceiving )
		{
			ReceivingResult = socket.receiveBlock( );
			if ( 0 < ReceivingResult )
			{
				std::unique_lock uLock( MutexRcvBuf );
				CvForResumingRcv.wait( uLock );
				// Reset
				ReceivingResult = -2;
			}
			else if ( ReceivingResult < 0 && WSAETIMEDOUT == WSAGetLastError() )
			{
				// Reset
				ReceivingResult = -2;
				// Trying to receive again
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

	SocketToServer = std::make_unique< Socket >( Socket::Type::TCP );
	mFPS_ = (uint32_t)(*glpService).vault( )[ HK_FORE_FPS ];
	FrameCount_interval = mFPS_;
	ASSERT_TRUE( -1 != SocketToServer->bind(EndPoint::Any) );
	// NOTE: Setting socket option should be done following binding it.
	DWORD timeout = 1000ul;
	if ( 0 != setsockopt(SocketToServer->handle(), SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, (int)sizeof(DWORD)) )
	{
		std::string msg( "setsockopt error: " );
		(*glpService).console( )->printFailure( FailureLevel::WARNING, msg+std::to_string(WSAGetLastError()) );
	}
	if ( char queueServerIPAddress[ ] = "192.168.219.102";
		 -1 == SocketToServer->connect(EndPoint(queueServerIPAddress, QUEUE_SERVER_PORT)) )
	{
		// Exception
		(*glpService).console( )->printFailure( FailureLevel::WARNING, "Failed to connect to the queue server.\n" );
		// Triggering
		++mFrameCount_disconnection;
	}
	else
	{
		(*glpService).console( )->print( "Succeeded to connect to the queue server.", sf::Color::Green );
		const Dword version = (*glpService).vault( ).at( HK_VERSION );
//TODO: OTP로 신원 확인
		// Make it possible to prevent a fake client (as hacking probably).
		std::string encryptedInvitation( std::to_string(util::hash::Digest(version+ADDITIVE)) );
		// Sending the digested, or encrypted version info for a client to be verified.
		if ( -1 == SocketToServer->sendBlock(encryptedInvitation.data(),(int)encryptedInvitation.size()) )
		{
#ifdef _DEBUG
			// Exception
			(*glpService).console( )->printFailure( FailureLevel::WARNING, "Failed to send the invitation to the queue server.\n" );
#endif
			// Triggering
			++mFrameCount_disconnection;
		}
		else
		{
			Thread0 = std::make_unique< std::thread >( &Receive, std::ref(*SocketToServer) );
		}
	}
	setScene( ::scene::online::ID::WAITING );
	loadResources( );

	IsInstantiated = true;
}

::scene::online::Online::~Online( )
{
	IsReceiving = false;
	IsSending = false;
	CvForResumingRcv.notify_one( );
	CvForResumingSnd.notify_one( );
	if ( nullptr != Thread0 && true == Thread0->joinable() )
	{
		Thread0->join( );
	}
	if ( nullptr != Thread1 && true == Thread1->joinable() )
	{
		Thread1->join( );
	}
	Thread0.reset( );
	Thread1.reset( );
	ZeroMemory( SendingBuffer, SND_BUF_SIZ );
	DataToSend = nullptr;
	DataSizeToSend = -1;
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
		(*glpService).console( )->printFailure( FailureLevel::FATAL, std::string("File Not Found: ")+scriptPathNName );
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
			(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName0.data(), scriptPathNName );
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
				(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK,
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
					(*glpService).console( )->printScriptError( ExceptionType::RANGE_CHECK,
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
				(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK,
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
					(*glpService).console( )->printScriptError( ExceptionType::RANGE_CHECK,
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
				(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK,
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
	const sf::Vector2i cast( mSpriteClipSize_ );
	mSprite.setTextureRect( sf::IntRect(0, cast.y, cast.x, cast.y) );
	mSprite.setPosition( (sf::Vector2f(mWindow_.getSize())-mSpriteClipSize_)*0.5f );
}

::scene::ID scene::online::Online::update( std::list<sf::Event>& eventQueue )
{
	::scene::ID retVal = ::scene::ID::AS_IS;
	if ( 0u == mFrameCount_disconnection )
	{
		if ( -1 == SendingResult )
		{
			// Exception
			++mFrameCount_disconnection;
			return ::scene::ID::AS_IS;
		}
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

bool scene::online::Online::hasReceived( )
{
	if ( 1u*mFPS_ <= FrameCount_interval && 0 < ReceivingResult )
	{
		//TODO
		ReceivingResult = -2;
		FrameCount_interval = 0u;
		return true;
	}
	else
	{
		if ( 0 == ReceivingResult || -1 == ReceivingResult )
		{
			(*glpService).console( )->printFailure( FailureLevel::FATAL, "Failed to receive." );
			// Triggering
			++mFrameCount_disconnection;
		}
		return false;
	}
}

std::optional<std::string> scene::online::Online::getByTag( const Tag tag, const uint8_t option )
{
	const char* const rcvBuf = SocketToServer->receivingBuffer( );
	std::string_view strView( rcvBuf );
	uint32_t begin = -1;
	const uint32_t tagLen = (uint32_t)std::strlen( tag );
	if ( 0u == (option & Option::FIND_END_TO_BEGIN) )
	{
		if ( const size_t pos = strView.find(tag);
			 std::string_view::npos != pos )
		{
			begin = (uint32_t)pos;
		}
	}
	else
	{
		if ( const size_t pos = strView.find_last_of(tag[0]);
			 std::string_view::npos != pos )
		{
			if ( 0 == strView.compare(pos, tagLen, tag) )
			{
				begin = (uint32_t)pos;
			}
		}
	}

	if ( -1 == begin )
	{
		return std::nullopt;
	}

	uint32_t end = -1;
	uint32_t begin2 = 0;
	if ( 0u != (option & Option::SERIALIZED) )
	{
		const uint32_t pos = begin + tagLen;
		const int size = std::atoi( &rcvBuf[pos] );
		if ( size_t pos2 = strView.find(TOKEN_SEPARATOR_2, pos);
			 std::string_view::npos != pos2 )
		{
			begin2 = (uint32_t)++pos2;
			end = (uint32_t)pos2 + size;
		}
	}
	else
	{
		if ( const size_t pos = strView.find(TOKEN_SEPARATOR, begin);
			 std::string_view::npos != pos )
		{
			end = (uint32_t)pos;
		}
	}

	ASSERT_TRUE( -1 != end );

	if ( 0u == (option & Option::RETURN_TAG_ATTACHED) )
	{
		begin = begin2;
	}

	return std::string( &strView[begin], end-begin );
}

#ifdef _DEV
::scene::ID scene::online::Online::currentScene( ) const
{
	return ::scene::ID::ONLINE_BATTLE;
}

void scene::online::Online::setScene( const::scene::online::ID nextSceneID )
{
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
#endif

void scene::online::Online::stopReceivingFromQueueServer( )
{
	IsReceiving = false;
	CvForResumingRcv.notify_one( );
	if ( true == Thread0->joinable() )
	{
		Thread0->join( );
	}
	Thread0.reset( );
}

bool scene::online::Online::connectToMainServer( )
{
	SocketToServer = std::make_unique< Socket >( Socket::Type::TCP );
	ASSERT_TRUE( -1 != SocketToServer->bind(EndPoint::Any) );
	// NOTE: Socket option should be set following binding it.
	DWORD timeout = 1000ul;
	if ( 0 != setsockopt(SocketToServer->handle(), SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, (int)sizeof(DWORD)) )
	{
		std::string msg( "setsockopt error: " );
		(*glpService).console( )->printFailure( FailureLevel::WARNING, msg+std::to_string(WSAGetLastError()) );
		return false;
	}
	if ( char mainServerIPAddress[ ] = "192.168.219.102";
		 -1 == SocketToServer->connect(EndPoint(mainServerIPAddress, MAIN_SERVER_PORT)) )
	{
		// Exception
		(*glpService).console( )->printFailure( FailureLevel::WARNING, "Failed to connect to the main server.\n" );
		// Triggering
		++mFrameCount_disconnection;
		return false;
	}
	(*glpService).console( )->print( "Succeeded to connect to the main server.", sf::Color::Green );
	return true;
}

void scene::online::Online::send( char* const data, const int size )
{
	if ( nullptr == Thread0 )
	{
		strncpy_s( SendingBuffer, SND_BUF_SIZ, data, size );
		DataSizeToSend = size;
		Thread0 = std::make_unique< std::thread >( &Send, std::ref(*SocketToServer), 1000 );
	}
	else
	{
		DataToSend = data;
		DataSizeToSend = size;
		CvForResumingSnd.notify_one( );
	}
}

void scene::online::Online::send( std::string& data )
{
	send( data.data(), (int)data.size() );
}

void scene::online::Online::disconnect( )
{
	// Triggering
	++mFrameCount_disconnection;
}

bool scene::online::Online::hasSent( )
{
	if ( 0 <= SendingResult )
	{
		//TODO
		SendingResult = -2;
		return true;
	}
	else
	{
		if ( -1 == SendingResult )
		{
			(*glpService).console( )->printFailure( FailureLevel::FATAL, "Failed to send." );
			// Triggering
			++mFrameCount_disconnection;
		}
		return false;
	}
}

void scene::online::Online::receive( )
{
	if ( nullptr == Thread1 )
	{
		Thread1 = std::make_unique< std::thread >( &Receive, std::ref(*SocketToServer) );
	}
	else
	{
		CvForResumingRcv.notify_one( );
	}
}

//char* const scene::online::Online::receivingBuffer( )
//{
//	return SocketToServer->receivingBuffer( );
//}