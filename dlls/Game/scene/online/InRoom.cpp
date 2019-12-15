#include "../../pch.h"
#include "InRoom.h"
#include "Online.h"
#include "../../ServiceLocatorMirror.h"
#include "../CommandList.h"
#include "../VaultKeyList.h"
#include <utility>

const uint8_t ROOM_CAPACITY = 4;

bool scene::online::InRoom::IsInstantiated = false;

scene::online::InRoom::InRoom( sf::RenderWindow& window, Online& net, const bool asHost )
	: mAsHost( asHost ), mIsReceiving( false ), mHasCanceled( false ), mFrameCount( 0 ),
	mWindow_( window ), mNet( net )
{
	mFPS_ = (int32_t)gService( )->vault( )[ HK_FORE_FPS ];
	const std::string& nickname = mNet.nickname( );
	mParticipants.reserve( ROOM_CAPACITY );
	mDigestedNickname_ = ::util::hash::Digest( nickname.data(), (uint8_t)nickname.size() );
	mParticipants.emplace( mDigestedNickname_, ::ui::PlayView(mWindow_, mNet) );
	loadResources( );
#ifdef _DEBUG
	gService( )->console( ).print( "Now in a room.", sf::Color::Green );
#endif
#ifdef _DEV
	gService( )->console( ).addCommand( CMD_LEAVE_ROOM, std::bind( &scene::online::InRoom::leaveRoom,
																 this, std::placeholders::_1 ) );
	if ( true == asHost )
	{
		gService( )->console( ).addCommand( CMD_START_GAME, std::bind( &scene::online::InRoom::startGame,
																	 this, std::placeholders::_1 ) );
	}
#endif
	IsInstantiated = true;
}

scene::online::InRoom::~InRoom( )
{
#ifdef _DEV
	if ( nullptr != gService() )
	{
		gService( )->console( ).removeCommand( CMD_LEAVE_ROOM );
		if ( true == mAsHost )
		{
			gService( )->console( ).removeCommand( CMD_START_GAME );
		}
	}
#endif
	IsInstantiated = false;
}

void scene::online::InRoom::loadResources( )
{
	mBackgroundRect.setSize( sf::Vector2f(mWindow_.getSize()) );
	const sf::Color SKY( 0x8ae5ffff );
	mBackgroundRect.setFillColor( SKY );
	sf::Vector2f playerViewPos( 130.f, 0.f );
	float cellSize = 30.f;
	if ( auto it = mParticipants.find(mDigestedNickname_);
		mParticipants.end() != it )
	{
		it->second.setDimension( playerViewPos, cellSize );
	}
}

::scene::online::ID scene::online::InRoom::update( std::list<sf::Event>& eventQueue )
{
	if ( false == mIsReceiving )
	{
		mNet.receive( );
		mIsReceiving = true;
	}
	
	bool hasToRespond = false;
	if ( true == mNet.hasReceived() )
	{
		mIsReceiving = false;
		if ( std::optional<std::string> response(mNet.getByTag(TAGGED_REQ_GET_READY,
																Online::Option::RETURN_TAG_ATTACHED));
			 std::nullopt != response )
		{
			mFrameCount = mFPS_*-3;
		}
		hasToRespond = true;
	}

	for ( auto& it : mParticipants )
	{
		hasToRespond = it.second.update( eventQueue );
	}

	if ( true == hasToRespond )
	{
		mNet.sendZeroByte( );
	}

	::scene::online::ID retVal = ::scene::online::ID::AS_IS;
	if ( true == mHasCanceled )
	{
		retVal = ::scene::online::ID::IN_LOBBY;
	}

	return retVal;
}

void scene::online::InRoom::draw( )
{
	mWindow_.draw( mBackgroundRect );
	for ( auto& it : mParticipants )
	{
		it.second.draw( mFrameCount/mFPS_-1 );
	}
	if ( 0 != mFrameCount )
	{
		++mFrameCount;
	}
}

void scene::online::InRoom::startGame( const std::string_view& arg )
{
	std::string request( TAGGED_REQ_START_GAME );
	mNet.send( request.data(), (int)request.size() );
}

void scene::online::InRoom::leaveRoom( const std::string_view& arg )
{
	std::string request( TAGGED_REQ_LEAVE_ROOM );
	mNet.send( request.data(), (int)request.size() );
	mHasCanceled = true;
}

