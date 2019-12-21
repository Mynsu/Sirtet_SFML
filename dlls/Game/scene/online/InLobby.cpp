#include "../../pch.h"
#include "InLobby.h"
#include "../../ServiceLocatorMirror.h"
#include "../CommandList.h"
#include "Online.h"
#include "../VaultKeyList.h"

const uint32_t USER_LIST_UPDATE_INTERVAL = 600;
const float ERROR_RANGE = 5.f;
const float SPEED = .5f;

bool scene::online::InLobby::IsInstantiated = false;

scene::online::InLobby::InLobby( sf::RenderWindow& window, ::scene::online::Online& net )
	: mIsReceiving( false ), mHasCanceled( false ), mHasJoined( false ),
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
#endif
	sf::Text tf( mNet.myNickname(), mFont_ );
	const sf::Vector2f center( mWindow_.getSize() );
	tf.setPosition(	center*.5f );
	tf.setFillColor( sf::Color(0xffa500ff) ); // Orange
	mUserList.emplace( mNet.myNickname(), std::make_pair(tf,0) );

	Packet packet;
	uint8_t ignored = 1;
	packet.pack( TAGGED_REQ_UPDATE_USER_LIST, ignored );
	mNet.send( packet );
	loadResources( );
	IsInstantiated = true;
}

scene::online::InLobby::~InLobby( )
{
#ifdef _DEV
	if ( nullptr != gService() )
	{
		gService( )->console( ).removeCommand( CMD_CREATE_ROOM );
		gService( )->console( ).removeCommand( CMD_CANCEL_CONNECTION );
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
	if ( false == mIsReceiving )
	{
		mNet.receive( );
		mIsReceiving = true;
	}

	::scene::online::ID retVal = ::scene::online::ID::AS_IS;
	bool hasToRespond = false;
	if ( true == mNet.hasReceived(1000u) )
	{
		hasToRespond = true;
		if ( std::optional<std::string> response(mNet.getByTag(TAGGED_REQ_CREATE_ROOM,
															   Online::Option::RETURN_TAG_ATTACHED,
															   sizeof(char)));
			std::nullopt != response )
		{
			retVal = ::scene::online::ID::IN_ROOM_AS_HOST;
		}
		else if ( std::optional<std::string> visitor(mNet.getByTag(TAGGED_NOTI_VISITOR,
																   Online::Option::INDETERMINATE_SIZE));
				 std::nullopt != visitor )
		{
			const std::string& otherNickname = visitor.value();
			sf::Text tf( otherNickname, mFont_ );
			const sf::Vector2f center( mWindow_.getSize() );
			tf.setPosition(	center*.5f );
			mUserList.emplace( otherNickname, std::make_pair(tf,0) );
		}
		else if ( std::optional<std::string> left(mNet.getByTag(TAGGED_NOTI_SOMEONE_LEFT,
															   Online::Option::INDETERMINATE_SIZE));
				 std::nullopt != left )
		{
#ifdef _DEBUG
			if ( 1 != mUserList.erase(left.value()) )
			{
				__debugbreak( );
			}
#else
			mUserList.erase( left.value() );
#endif
		}
		else if ( std::optional<std::string> userList(mNet.getByTag(TAGGED_REQ_UPDATE_USER_LIST,
															   Online::Option::INDETERMINATE_SIZE));
			std::nullopt != userList )
		{
			const std::string& _userList( userList.value() );
			size_t curPos = 0;
			size_t nextPos = _userList.find(TOKEN_SEPARATOR_2);
			const sf::Vector2f offset( 10.f, 0.f );
			float mul = 0.f;
			while ( _userList.npos != nextPos )
			{
				const std::string otherNickname( _userList.substr(curPos, nextPos-curPos) );
				sf::Text tf( otherNickname, mFont_ );
				const sf::Vector2f center( mWindow_.getSize() );
				tf.setPosition( center*.5f + offset*mul );
				mUserList.emplace( otherNickname, std::make_pair(tf, 0) );
				curPos = nextPos + 1;
				nextPos = _userList.find(TOKEN_SEPARATOR_2, curPos);
				mul += 1.f;
			}
		}
		mIsReceiving = false;
	}

	if ( true == hasToRespond )
	{
		mNet.sendZeroByte( );
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
}

void scene::online::InLobby::cancelConnection( const std::string_view& )
{
	//TODO
	mHasCanceled = true;
}

void scene::online::InLobby::createRoom( const std::string_view& )
{
	//TODO
	std::string request( TAGGED_REQ_CREATE_ROOM );
	mNet.send( request.data(), (int)request.size() );
}