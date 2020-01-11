#include "../../pch.h"
#include "InLobby.h"
#include "../../ServiceLocatorMirror.h"
#include "../CommandList.h"
#include "Online.h"
#include "../VaultKeyList.h"

const uint32_t UPDATE_INTERVAL = 300;
const float ERROR_RANGE = 5.f;
const float SPEED = .5f;

bool scene::online::InLobby::IsInstantiated = false;
bool scene::online::InLobby::BinarySemaphore = false;

scene::online::InLobby::InLobby( sf::RenderWindow& window, ::scene::online::Online& net )
	: mIsReceiving( false ), mHasCanceled( false ), mHasJoined( false ),
	mFrameCount_update( 30 ),
	mWindow_( window ), mNet( net )
{
#ifdef _DEBUG
	gService( )->console( ).print( "Now in the lobby.", sf::Color::Green );
#endif
#ifdef _DEV
	gService( )->console( ).addCommand( CMD_CREATE_ROOM, std::bind(&scene::online::InLobby::createRoom,
																	  this, std::placeholders::_1) );
	gService( )->console( ).addCommand( CMD_CANCEL_CONNECTION, std::bind(&scene::online::InLobby::cancelConnection,
																			this, std::placeholders::_1) );
	gService( )->console( ).addCommand( CMD_JOIN_ROOM, std::bind(&scene::online::InLobby::joinRoom,
																		 this, std::placeholders::_1) );
#endif
	sf::Text tf( mNet.myNickname(), mFont_ );
	const sf::Vector2f center( mWindow_.getSize() );
	// 궁금: 어떻게 겹치지 않게 할까?
	tf.setPosition(	center*.4f );
	tf.setFillColor( sf::Color(0xffa500ff) ); // Orange
	mUserList.emplace( mNet.myNickname(), std::make_pair(tf,0) );
	loadResources( );
	BinarySemaphore = false;
	IsInstantiated = true;
}

scene::online::InLobby::~InLobby( )
{
#ifdef _DEV
	if ( nullptr != gService() )
	{
		gService( )->console( ).removeCommand( CMD_CREATE_ROOM );
		gService( )->console( ).removeCommand( CMD_CANCEL_CONNECTION );
		gService( )->console( ).removeCommand( CMD_JOIN_ROOM );
	}
#endif
	IsInstantiated = false;
}

void scene::online::InLobby::loadResources( )
{
	const sf::Vector2f winSize( mWindow_.getSize() );
	mBackground.setSize( winSize );
	mBackground.setFillColor( sf::Color::Cyan );
	mUserNicknamesBox.setSize( winSize*.8f );
	mUserNicknamesBox.setOrigin( winSize*.4f );
	mUserNicknamesBox.setPosition( winSize*.5f );
	mUserNicknamesBox.setFillColor( sf::Color::Black );
	mUserNicknamesBox.setOutlineThickness( 5.f );
	mUserNicknamesBox.setOutlineColor( sf::Color(0x0000007f) );
	mFont_.loadFromFile( "Fonts/AGENCYR.TTF" );
	mMovingPoint[ 0 ] = sf::Vector2f( 100.f, 500.f );
	mMovingPoint[ 1 ] = sf::Vector2f( 100.f, 100.f );
	mMovingPoint[ 2 ] = sf::Vector2f( 500.f, 100.f );
	mMovingPoint[ 3 ] = sf::Vector2f( 500.f, 500.f );
}

::scene::online::ID scene::online::InLobby::update( std::list<sf::Event>& eventQueue )
{
	::scene::online::ID retVal = ::scene::online::ID::AS_IS;
	if ( true == mNet.hasReceived() )
	{
		mIsReceiving = false;
		if ( std::optional<std::string> resultCreatingRoom(mNet.getByTag(TAGGED_REQ_CREATE_ROOM,
															   Online::Option::RETURN_TAG_ATTACHED,
															   sizeof(char)));
			std::nullopt != resultCreatingRoom )
		{
			BinarySemaphore = false;
			retVal = ::scene::online::ID::IN_ROOM_AS_HOST;
		}
		else if ( std::optional<std::string> resultJoiningRoom(mNet.getByTag(TAGGED_REQ_JOIN_ROOM,
																			 Online::Option::SPECIFIED_SIZE,
																			 sizeof(uint8_t)));
				 std::nullopt != resultJoiningRoom )
		{
			BinarySemaphore = false;
			const ResultJoiningRoom res = (ResultJoiningRoom)*resultJoiningRoom.value().data();
			switch( res )
			{
				case ResultJoiningRoom::FAILED_BY_SERVER_ERROR:
					gService()->console().print( "Failed to join the room by an error.", sf::Color::Green );
					break;
				case ResultJoiningRoom::FAILED_BY_FULL_ROOM:
					gService()->console().print( "Room is full.", sf::Color::Green );
					break;
				case ResultJoiningRoom::SUCCCEDED:
					retVal = ::scene::online::ID::IN_ROOM;
					break;
				case ResultJoiningRoom::FAILED_DUE_TO_TARGET_NOT_CONNECTING:
					gService()->console().print( "That nicknamed-user is not connecting.", sf::Color::Green );
					break;
				case ResultJoiningRoom::FAILED_DUE_TO_SELF_TARGET:
					gService()->console().print( "It doesn't make any sense to join yourself.", sf::Color::Green );
					break;
				default:
#ifdef _DEBUG
					__debugbreak( );
#else
					__assume( 0 );
#endif
					break;
			}
		}
		
		if ( std::optional<std::string> userList(mNet.getByTag(TAGGED_REQ_USER_LIST_IN_LOBBY,
															   Online::Option::INDETERMINATE_SIZE, NULL));
			std::nullopt != userList )
		{
			const std::string& _userList( userList.value() );
			const char* const ptr = _userList.data();
			const uint32_t userListSize = (uint32_t)_userList.size();
			uint32_t curPos = 0;
			// NOTE: Faster than std::set or std::unordered_set.
			std::vector<std::string> curUsers;
			while ( userListSize != curPos )
			{
				const uint8_t curSize = ptr[curPos];
				++curPos;
				const std::string otherNickname( _userList.substr(curPos, curSize) );
				curUsers.emplace_back( otherNickname );
				curPos += curSize;
			}
			for ( auto it = mUserList.begin(); mUserList.end() != it; )
			{
				bool hasFound = false;
				for ( auto it2 = curUsers.cbegin(); curUsers.cend() != it2; ++it2 )
				{
					if ( it->first == *it2 )
					{
						hasFound = true;
						curUsers.erase( it2 );
						break;
					}
				}
				if ( false == hasFound )
				{
					it = mUserList.erase(it);
				}
				else
				{
					++it;
				}
			}
			const sf::Vector2f center( mWindow_.getSize() );
			const sf::Vector2f offset( 20.f, 0.f );
			float mul = 0.f;
			for ( const std::string& nickname : curUsers )
			{
				sf::Text tf( nickname, mFont_ );
				tf.setPosition( center*.5f + offset*mul );
				mUserList.emplace( nickname, std::make_pair(tf, 0) );
				mul += 1.f;
			}
		}
	}
	
	if ( false == mIsReceiving )
	{
		mNet.receive( );
		mIsReceiving = true;
	}

	if ( UPDATE_INTERVAL <= mFrameCount_update )
	{
		mFrameCount_update = 0;
		std::string req( TAGGED_REQ_USER_LIST_IN_LOBBY );
		mNet.send( req.data(), (int)req.size() );
	}
	
	if ( true == mHasCanceled )
	{
		retVal = ::scene::online::ID::MAIN_MENU;
	}

	return retVal;
}

void scene::online::InLobby::draw( )
{
	mWindow_.draw( mBackground );
	mWindow_.draw( mUserNicknamesBox );
	for ( auto& it : mUserList )
	{
		while ( true )
		{
			auto& tf = it.second;
			const sf::Vector2f dir( mMovingPoint[tf.second] - tf.first.getPosition() );
			Vector<2> v( dir.x, dir.y );
			const float mag = v.magnitude();
			if ( mag <= ERROR_RANGE )
			{
				if ( MOVING_POINT_NUM-1 == tf.second )
				{
					tf.second = 0;
				}
				else
				{
					++tf.second;
				}
			}
			else
			{
				const auto& vault =	gService()->vault();
				bool hasFocused = true;
				if ( const auto it = vault.find(HK_HAS_GAINED_FOCUS);
					vault.cend() != it )
				{
					hasFocused = (bool)it->second;
				}
#ifdef _DEBUG
				else
				{
					__debugbreak();
				}
#endif
				float mul = SPEED;
				if ( false == hasFocused )
				{
					uint32_t foreFPS = 60;
					uint32_t backFPS = 30;
					if ( const auto it = vault.find(HK_FORE_FPS);
						vault.cend() != it )
					{
						foreFPS = it->second;
					}
#ifdef _DEBUG
					else
					{
						__debugbreak();
					}
#endif
					if ( const auto it = vault.find(HK_BACK_FPS);
						vault.cend() != it )
					{
						backFPS = it->second;
					}
#ifdef _DEBUG
					else
					{
						__debugbreak();
					}
#endif
					mul *= (float)(foreFPS/backFPS);
				}
				const Vector<2> nv( v.normalize()*mul );
				tf.first.move( nv.mComponents[0], nv.mComponents[1] );
				mWindow_.draw( tf.first );
				break;
			}
		}
	}
	++mFrameCount_update;
}

void scene::online::InLobby::cancelConnection( const std::string_view& )
{
	mHasCanceled = true;
}

void scene::online::InLobby::createRoom( const std::string_view& )
{
	if ( true == BinarySemaphore )
	{
		gService()->console().print( "Retry to create a room later.", sf::Color::Green );
		return;
	}
	BinarySemaphore = true;
	std::string request( TAGGED_REQ_CREATE_ROOM );
	mNet.send( request.data(), (int)request.size() );
}

void scene::online::InLobby::joinRoom( const std::string_view& arg )
{
	if ( true == BinarySemaphore )
	{
		gService()->console().print( "Retry to join the room later.", sf::Color::Green );
		return;
	}
	BinarySemaphore = true;
	// Exception
	if ( const size_t pos = arg.find_first_of(' ');
		arg.npos != pos	)
	{
		gService()->console().print( "User's nickname shouldn't have a space." );
		return;
	}
	else if ( arg == mNet.myNickname() )
	{
		gService()->console().print( "It doesn't make any sense to join yourself." );
		return;
	}
	Packet packet;
	std::string otherNickname( arg );
	packet.pack( TAGGED_REQ_JOIN_ROOM, otherNickname );
	mNet.send( packet );
}
