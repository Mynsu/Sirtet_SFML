#include "../../pch.h"
#include "Lobby.h"
#include "../../ServiceLocatorMirror.h"
#include <lua.hpp>

bool ::scene::online::Lobby::IsInstantiated = false;

namespace
{
	constexpr char TOKEN_SEPARATOR = ' ';
	constexpr char TAG_INVITATION[ ] = "inv:";
	// Recommended to be renewed periodically for security.
	const int ADDITIVE = 1246;
	constexpr char TAG_TICKET[ ] = "t:";
	constexpr uint8_t TAG_TICKET_LEN = ::util::hash::Measure( TAG_TICKET );
	constexpr char TAG_ORDER_IN_QUEUE[ ] = "qL:";
	constexpr uint8_t TAG_ORDER_IN_QUEUE_LEN = ::util::hash::Measure( TAG_ORDER_IN_QUEUE );
	const uint16_t QUEUE_SERVER_PORT = 10000u;
	const uint16_t MAIN_SERVER_PORT = 54321u;
	int SendingResult = -2;
	int ReceivingResult = -2;
	// Set false to stop the reception from the queue server( and start that from the main server.)
	bool IsReceiving = true;
	std::unique_ptr<std::thread> Thread;
	std::condition_variable CvUpdated;
	std::mutex MutexRcvBuf;

	void Send( Socket& socket, char* const data, const int length, const uint32_t waitForMs = 1000u )
	{
		std::this_thread::sleep_for( std::chrono::milliseconds(waitForMs) );
		SendingResult = socket.sendBlock( data, length );
	}
	void Receive( Socket& socket )
	{
		IsReceiving = true;
		while ( true == IsReceiving )
		{
			ReceivingResult = socket.receiveBlock( );
			if ( 0 == ReceivingResult || ReceivingResult < 0 && WSAETIMEDOUT != WSAGetLastError() )
			{
				break;
			}
			else if ( 0 < ReceivingResult )
			{
				std::unique_lock uLock( MutexRcvBuf );
				CvUpdated.wait( uLock );
			}
		}
	}
}

scene::online::Lobby::Lobby( sf::RenderWindow& window, const SetScene_t& setScene )
	: mIsWaitingForTicketVerification( false ),
	mFrameCount_disconnection( 0u ), mFrameCount_receivingInterval_( 0u ),
	mMyOrderInQueueLine( MAXINT32 ),
	mSocketToQueueServer( std::make_unique<Socket>(Socket::Type::TCP) ),
	mSocketToMainServer( std::make_unique<Socket>(Socket::Type::TCP) ),
	mWindow_( window ), mSetScene( setScene )
{
	ASSERT_FALSE( IsInstantiated );

	constexpr HashedKey HK_FORE_FPS = ::util::hash::Digest( "foreFPS", 7 );
	mFPS_ = static_cast<uint32_t>((*glpService).vault( )[ HK_FORE_FPS ]);

	ASSERT_TRUE( -1 != mSocketToQueueServer->bind(EndPoint::Any) );
	// NOTE: Setting socket option should be done following binding it.
	DWORD timeout = 1000ul;
	if ( 0 != setsockopt(mSocketToQueueServer->handle(), SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, (int)sizeof(DWORD)) )
	{
		std::string msg( "setsockopt error: " );
		(*glpService).console( )->printFailure( FailureLevel::FATAL, msg+std::to_string(WSAGetLastError()) );
	}
	if ( char queueServerIPAddress[ ] = "192.168.219.102";
		 -1 == mSocketToQueueServer->connect(EndPoint(queueServerIPAddress, QUEUE_SERVER_PORT)) )
	{
		// Exception
		(*glpService).console( )->printFailure( FailureLevel::WARNING, "Connection to the queue server failed.\n" );
		// Triggering
		++mFrameCount_disconnection;
	}
	else
	{
		(*glpService).console( )->print( "Connection to the queue server succeeded.", sf::Color::Green );
		constexpr HashedKey HK_VERSION = util::hash::Digest( "version", 7 );
		const Dword version = (*glpService).vault( ).at( HK_VERSION );
		// Make it possible to prevent a fake client (as hacking probably).
		std::string encryptedInvitation( TAG_INVITATION + std::to_string(util::hash::Digest(version+ADDITIVE)) );
		// Sending the digested, or encrypted version info for a client to be verified.
		if ( -1 == mSocketToQueueServer->sendBlock(encryptedInvitation.data(),(int)encryptedInvitation.size()) )
		{
#ifdef _DEBUG
			// Exception
			(*glpService).console( )->printFailure( FailureLevel::WARNING, "Sending the invitation to the queue server failed.\n" );
#endif
			// Triggering
			++mFrameCount_disconnection;
		}
		else
		{
			Thread = std::make_unique< std::thread >( &Receive, std::ref(*mSocketToQueueServer) );
			//Triggering, waiting
			++mFrameCount_receivingInterval_;
		}
	}
	loadResources( );

	IsInstantiated = true;
}

::scene::online::Lobby::~Lobby( )
{
	IsReceiving = false;
	CvUpdated.notify_one( );
	if ( nullptr != Thread && true == Thread->joinable() )
	{
		Thread->join( );
	}

	SendingResult = -2;
	ReceivingResult = -2;
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
}

void ::scene::online::Lobby::update( std::list<sf::Event>& eventQueue )
{
	if ( true == mIsWaitingForTicketVerification )
	{
		// Reset
		mIsWaitingForTicketVerification = false;
		if ( 0 < SendingResult )
		{
//TODO: 로비 만들기
		}
		else if ( -1 == SendingResult )
		{
#ifdef _DEBUG
			// Exception
			(*glpService).console( )->printFailure( FailureLevel::WARNING, "Sending the ticket to the main server failed.\n" );
#endif
			// Triggering
			++mFrameCount_disconnection;
		}
		else if ( 0 == SendingResult )
		{
#ifdef _DEBUG
			(*glpService).console( )->printFailure( FailureLevel::WARNING, "Ticket verification failed.\n" );
#endif
			// Triggering
			++mFrameCount_disconnection;
		}
		// Reset
		SendingResult = -2;
	}
	// Checking if a message has arrived or not every 1 second.
	else if ( 1u*mFPS_ <= mFrameCount_receivingInterval_ && 0 < ReceivingResult )
	{
		char* const rcvBuf = mSocketToQueueServer->receivingBuffer( );
		std::string_view strView( rcvBuf );
		// When having received a ticket for the main server,
		if ( const size_t pos = strView.find(TAG_TICKET); std::string_view::npos != pos )
		{
			// Reset
			mFrameCount_receivingInterval_ = 0u;
			IsReceiving = false;
			CvUpdated.notify_one( );
			if ( true == Thread->joinable() )
			{
				Thread->join( );
			}

			ASSERT_TRUE( -1 != mSocketToMainServer->bind(EndPoint::Any) );
			if ( char mainServerIPAddress[ ] = "192.168.219.102";
					-1 == mSocketToMainServer->connect(EndPoint(mainServerIPAddress, MAIN_SERVER_PORT)) )
			{
				// Exception
				(*glpService).console( )->printFailure( FailureLevel::WARNING, "Connection to the main server failed.\n" );
				// Triggering
				++mFrameCount_disconnection;
			}
			else
			{
				(*glpService).console( )->print( "Connection to main server succeeded.", sf::Color::Green );
				const size_t end = strView.find( TOKEN_SEPARATOR, pos );
				ASSERT_TRUE( std::string_view::npos != end );
				std::string ticket( strView.substr(pos,end-pos) );
				// Sending to the main server the ticket, which the main server will verify.
				mIsWaitingForTicketVerification = true;
				Thread = std::make_unique< std::thread >( &Send, std::ref(*mSocketToMainServer),
														   &rcvBuf[pos], (int)(end-pos+1), 1000u );
			}
		}
		// When having received only the updated order(s) in the waiting queue line without any ticket for the main server,
		else if ( const size_t pos = strView.find_last_of( TAG_ORDER_IN_QUEUE[0] );
				  std::string_view::npos != pos && 0 == strView.substr(pos,TAG_ORDER_IN_QUEUE_LEN).compare(TAG_ORDER_IN_QUEUE) )
		{
			// Reset
			mFrameCount_receivingInterval_ = 1u;

			const size_t end = strView.find( TOKEN_SEPARATOR, pos );
			ASSERT_TRUE( std::string_view::npos != end );
			const size_t len = end - (pos+TAG_ORDER_IN_QUEUE_LEN);
			const std::string updated( strView.substr(pos+TAG_ORDER_IN_QUEUE_LEN,len) );
			mMyOrderInQueueLine = (int32_t)std::atoi( updated.data() );
			// Resuming reception
			CvUpdated.notify_one( );
#ifdef _DEBUG
			std::string msg( "My order in the queue line: " );
			(*glpService).console( )->print( msg+std::to_string(mMyOrderInQueueLine), sf::Color::Green );
#endif
		}
		else
		{
			(*glpService).console( )->printFailure( FailureLevel::FATAL, "Unknown message from the queue server." );
			// Triggering
			++mFrameCount_disconnection;
		}
	}
	else if ( 3u*mFPS_ < mFrameCount_disconnection )
	{
		mSetScene( ::scene::ID::MAIN_MENU );
		return;
	}
	else if ( 0==ReceivingResult || ReceivingResult<0 && WSAETIMEDOUT != WSAGetLastError() )
	{
		(*glpService).console( )->printFailure( FailureLevel::FATAL, "Disconnected from the queue server." );
		// Triggering
		++mFrameCount_disconnection;
	}
}

void ::scene::online::Lobby::draw( )
{
	const sf::Vector2i cast( mSpriteClipSize_ );
	if ( true == mIsWaitingForTicketVerification )
	{
		// "Connecting"
		mSprite.setTextureRect( sf::IntRect(0, 0, cast.x, cast.y) );
	}
	//else if ( 1u*mFPS_ == mFrameCount_receivingInterval_ )
	//{
		//mFrameCount_receivingInterval_ = 1u;
//TODO: 대기열 순서
		//setTextureRect
	//}
	else if ( 0u != mFrameCount_receivingInterval_ )
	{
		++mFrameCount_receivingInterval_;
	}
	else if ( 0u != mFrameCount_disconnection )
	{
		// "Connection Failed"
		mSprite.setTextureRect( sf::IntRect(0, cast.y, cast.x, cast.y) );
		++mFrameCount_disconnection;
	}
	mWindow_.draw( mSprite );
}

::scene::ID scene::online::Lobby::currentScene( ) const
{
	return ::scene::ID::ONLINE_BATTLE;//TODO
}
