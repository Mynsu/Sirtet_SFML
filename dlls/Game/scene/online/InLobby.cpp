#include "../../pch.h"
#include "InLobby.h"
#include "../../ServiceLocatorMirror.h"
#include "../CommandList.h"
#include "Online.h"

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
	//TODO
}

::scene::online::ID scene::online::InLobby::update( std::list<sf::Event>& eventQueue )
{
	if ( false == mIsReceiving )
	{
		mNet.receive( );
		mIsReceiving = true;
	}

	::scene::online::ID retVal = ::scene::online::ID::AS_IS;
	if ( true == mNet.hasReceived(1000u) )
	{
		if ( std::optional<std::string> response(mNet.getByTag(TAG_REQUEST,
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
	//TODO
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