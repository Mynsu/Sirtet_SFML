#include "../../pch.h"
#include "InLobby.h"
#include "../../ServiceLocatorMirror.h"
#include "../CommandList.h"
#include "Online.h"

const uint32_t USER_LIST_UPDATE_INTERVAL = 600;
const float ERROR_RANGE = 5.f;
const float SPEED = 1.f;

bool scene::online::InLobby::IsInstantiated = false;

scene::online::InLobby::InLobby( sf::RenderWindow& window, ::scene::online::Online& net )
	: mIsReceiving( false ), mHasCanceled( false ), mHasJoined( false ),
	mFrameCount_userListUpdate( 600 ),
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

	if ( USER_LIST_UPDATE_INTERVAL < mFrameCount_userListUpdate )
	{
		mFrameCount_userListUpdate = 0;
		Packet packet;
		uint8_t ignored = 1;
		packet.pack( TAGGED_REQ_USER_LIST, ignored );
		mNet.send( packet );
	}

	::scene::online::ID retVal = ::scene::online::ID::AS_IS;
	if ( true == mNet.hasReceived(1000u) )
	{
		if ( std::optional<std::string> userList(mNet.getByTag(TAGGED_REQ_USER_LIST,
															   Online::Option::INDETERMINATE_SIZE));
			std::nullopt != userList )
		{
			mUserList.clear( );
			mUserNicknameTextFields.clear( );
			std::string& _userList( userList.value() );
			size_t curPos = 0;
			size_t nextPos = _userList.find(TOKEN_SEPARATOR_2);
			const sf::Vector2f offset( 10.f, 0.f );
			float repeat = 0.f;
			while ( _userList.npos != nextPos )
			{
				mUserList.emplace_back( _userList.substr(curPos, nextPos-curPos) );
				std::string& one( mUserList.back() );
				sf::Text tf( one, mFont_ );
				if ( mNet.nickname() == one )
				{
					tf.setFillColor( sf::Color::Yellow );
					tf.setStyle( tf.Bold );
				}
				tf.setPosition( mMovingPoint[0] + offset*repeat );
				mUserNicknameTextFields.emplace_back( std::make_pair(tf, 0) );
				curPos = nextPos + 1;
				nextPos = _userList.find(TOKEN_SEPARATOR_2, curPos);
				repeat += 1.f;
			}
		}

		if ( std::optional<std::string> response(mNet.getByTag(TAGGED_REQ_CREATE_ROOM,
															   Online::Option::RETURN_TAG_ATTACHED,
															   sizeof(char)));
			std::nullopt != response )
		{
			retVal = ::scene::online::ID::IN_ROOM_AS_HOST;
		}
		mIsReceiving = false;
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
	for ( auto& it : mUserNicknameTextFields )
	{
		while ( true )
		{
			const sf::Vector2f dir( mMovingPoint[it.second] - it.first.getPosition() );
			Vector<2> v( dir.x, dir.y );
			const float mag = v.magnitude();
			if ( mag <= ERROR_RANGE )
			{
				if ( MOVING_POINT_NUM-1 == it.second )
				{
					it.second = 0;
				}
				else
				{
					++it.second;
				}
			}
			else
			{
				const Vector<2> nv( v.normalize()*SPEED );
				it.first.move( nv.mComponents[0], nv.mComponents[1] );
				mWindow_.draw( it.first );
				break;
			}
		}
	}
	++mFrameCount_userListUpdate;
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