#include "../../pch.h"
#include "Waiting.h"
#include "Online.h"
#include "../../ServiceLocatorMirror.h"
#include "../VaultKeyList.h"
#include "../CommandList.h"

#include <iostream>

bool scene::online::Waiting::IsInstantiated = false;

scene::online::Waiting::Waiting( sf::RenderWindow& window, Online& net )
	: mHasCanceled( false ), mIsReceiving( false ), mOrder( 0u ),
	mState( ::scene::online::Waiting::State::TICKETING ),
	mWindow_( window ), mNet( net )
{
#ifdef _DEV
	gService( )->console( ).addCommand( CMD_CANCEL_CONNECTION, std::bind(&::scene::online::Waiting::cancelConnection,
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
				if ( std::optional<std::string> ticket = mNet.getByTag(TAG_TICKET,
																	   Online::Option::RETURN_TAG_ATTACHED);
					 std::nullopt != ticket )
				{
#ifdef _DEBUG
					gService( )->console( ).print( ticket.value(), sf::Color::Green );
#endif
					// Reset
					mNet.stopReceivingFromQueueServer( );

					if ( true == mNet.connectToMainServer() )
					{
						// Sending to the main server the ticket, which the main server will verify.
						mNet.send( ticket.value() );
						mState = State::SUBMITTING_TICKET;
					}
				}
				// When having received only the updated order(s) in the waiting queue line without any ticket for the main server,
				else if ( std::optional<std::string> order = mNet.getByTag(TAG_ORDER_IN_QUEUE,
																		 Online::Option::FIND_END_TO_BEGIN);
						  std::nullopt != order )
				{
					mOrder = (int32_t)std::atoi( order.value().data() );
					mNet.receive( );
#ifdef _DEBUG
					std::string msg( "My order in the queue line: " );
					gService( )->console( ).print( msg+std::to_string(mOrder), sf::Color::Green );
#endif
				}
				else
				{
					gService( )->console( ).printFailure( FailureLevel::FATAL, "Unknown message from the queue server." );
					mNet.disconnect( );
				}
			}
			break;
		case State::SUBMITTING_TICKET:
			if ( false == mIsReceiving )
			{
				mNet.receive( );
				mIsReceiving = true;
			}
			else if ( true == mNet.hasReceived() )
			{
				if ( std::optional<std::string> nickname( mNet.getByTag(TAG_MY_NICKNAME, Online::Option::NONE) );
					 std::nullopt != nickname )
				{
#ifdef _DEBUG
					gService( )->console( ).print( nickname.value(), sf::Color::Green );
#endif
					mNet.setNickname( nickname.value() );
					retVal = ::scene::online::ID::IN_LOBBY;
				}
				else
				{
					mNet.disconnect( );
				}
				mIsReceiving = false;
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
//TODO: �ӽ�
}

void scene::online::Waiting::cancelConnection( const std::string_view& )
{
	mHasCanceled = true;
}