#include "../../pch.h"
#include "Waiting.h"
#include "Online.h"
#include "../../ServiceLocatorMirror.h"
#include "../VaultKeyList.h"
#include "../CommandList.h"

#include <iostream>

bool scene::online::Waiting::IsInstantiated = false;

scene::online::Waiting::Waiting( sf::RenderWindow& window, Online& net )
	: mHasCanceled( false ), mOrder( 0 ),
	mState( ::scene::online::Waiting::State::TICKETING ),
	mWindow_( window ), mNet( net )
{
#ifdef _DEV
	gService( )->console( ).addCommand( CMD_CANCEL_CONNECTION,
									   std::bind(&::scene::online::Waiting::cancelConnection,
										this, std::placeholders::_1) );
#endif
	IsInstantiated = true;
}

scene::online::Waiting::~Waiting( )
{
#ifdef _DEV
	if ( nullptr != gService() )
	{
		gService( )->console( ).removeCommand( CMD_CANCEL_CONNECTION );
	}
#endif
	IsInstantiated = false;
}

void scene::online::Waiting::loadResources( )
{
	//TODO
}

::scene::online::ID scene::online::Waiting::update( std::list<sf::Event>& eventQueue )
{
	::scene::online::ID retVal = ::scene::online::ID::AS_IS;
	switch ( mState )
	{
		case State::TICKETING:
			if ( true == mNet.hasReceived() )
			{
				if ( std::optional<std::string> ticket( mNet.getByTag(TAG_TICKET,
																	   Online::Option::RETURN_TAG_ATTACHED,
																	   sizeof(uint32_t)) );
					 std::nullopt != ticket )
				{
					std::string& _ticket = ticket.value();
#ifdef _DEBUG
					const char* const ptr = _ticket.data();

					std::string ticketID( std::to_string(::ntohl(*(HashedKey*)&ptr[std::strlen(TAG_TICKET)])) );
					gService( )->console( ).print( ticketID, sf::Color::Green );
#endif
					if ( true == mNet.connectToMainServer() )
					{
						// Sending to the main server the ticket, which the main server will verify.
						mNet.send( _ticket.data(), (int)_ticket.size() );
						mState = State::SUBMITTING_TICKET;
					}
				}
				else if ( std::optional<std::string> order( mNet.getByTag(TAG_ORDER_IN_QUEUE,
																		 Online::Option::FIND_END_TO_BEGIN,
																		   sizeof(uint32_t)) );
						  std::nullopt != order )
				{
					mOrder = ::ntohl(*(uint32_t*)order.value().data());
#ifdef _DEBUG
					std::string msg( "My order in the queue line: " );
					gService( )->console( ).print( msg+std::to_string(mOrder), sf::Color::Green );
#endif
				}
				else
				{
					gService( )->console( ).printFailure( FailureLevel::FATAL,
														 "Unknown message from the queue server." );
					mNet.disconnect( );
					break;
				}
				mNet.receive( );
			}
			break;
		case State::SUBMITTING_TICKET:
			if ( true == mNet.hasReceived() )
			{
				if ( std::optional<std::string> nickname( mNet.getByTag(TAG_MY_NICKNAME,
																		Online::Option::DEFAULT,
																		-1) );
					 std::nullopt != nickname )
				{
#ifdef _DEBUG
					gService( )->console( ).print( nickname.value(), sf::Color::Green );
#endif
					mNet.setMyNickname( nickname.value() );
					retVal = ::scene::online::ID::IN_LOBBY;
				}
				else
				{
					mNet.disconnect( );
				}
			}
			break;
		default:
#ifdef _DEBUG
			__debugbreak( );
#else
			__assume(0);
#endif
			break;
	}

	if ( true == mHasCanceled )
	{
		retVal = ::scene::online::ID::MAIN_MENU;
	}

	return retVal;
}

void scene::online::Waiting::draw( )
{
	//TODO
}

void scene::online::Waiting::cancelConnection( const std::string_view& )
{
	mHasCanceled = true;
}
