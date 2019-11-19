#include "../../pch.h"
#include "Waiting.h"
#include "Online.h"
#include "../../ServiceLocatorMirror.h"
#include "../VaultKeyList.h"
#include "../CommandList.h"

scene::online::Waiting::Waiting( sf::RenderWindow& window, Online& net )
	: mHasCanceled( false ), mOrder( 0u ),
	mState( ::scene::online::Waiting::State::TICKETING ),
	mWindow_( window ), mNet( net )
{
#ifdef _DEV
	(*glpService).console( )->addCommand( CMD_CANCEL_CONNECTION, std::bind(&::scene::online::Waiting::cancelConnection,
																			this, std::placeholders::_1) );
#endif
}

scene::online::Waiting::~Waiting( )
{
#ifdef _DEV
	(*glpService).console( )->removeCommand( CMD_CANCEL_CONNECTION );
#endif
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
				if ( std::optional<std::string> res = mNet.getByTag(TAG_TICKET,
													Online::Option::FIND_END_TO_BEGIN|Online::Option::RETURN_TAG_ATTACHED);
					 std::nullopt != res )
				{
#ifdef _DEBUG
					(*glpService).console( )->print( res.value(), sf::Color::Green );
#endif
					// Reset
					mNet.stopReceivingFromQueueServer( );

					if ( true == mNet.connectToMainServer() )
					{
						// Sending to the main server the ticket, which the main server will verify.
						mNet.send( res.value() );
						mState = State::SUBMITTING_TICKET;
					}
				}
				// When having received only the updated order(s) in the waiting queue line without any ticket for the main server,
				else if ( std::optional<std::string> res = mNet.getByTag(TAG_ORDER_IN_QUEUE, 0u);
						  std::nullopt != res )
				{
					mOrder = (int32_t)std::atoi( res.value().data() );
					mNet.receive( );
#ifdef _DEBUG
					std::string msg( "My order in the queue line: " );
					(*glpService).console( )->print( msg+std::to_string(mOrder), sf::Color::Green );
#endif
				}
				else
				{
					(*glpService).console( )->printFailure( FailureLevel::FATAL, "Unknown message from the queue server." );
					mNet.disconnect( );
				}
			}
			break;
		case State::SUBMITTING_TICKET:
			if ( true == mNet.hasSent() )
			{
				mNet.receive( );
			}
			else if ( true == mNet.hasReceived() )
			{
				if ( std::optional<std::string> nickname( mNet.getByTag(TAG_NICKNAME, 0) );
					 std::nullopt != nickname )
				{
#ifdef _DEBUG
					(*glpService).console( )->print( nickname.value(), sf::Color::Green );
#endif
					mNet.setNickname( nickname.value() );
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
	switch ( mState )
	{
		case State::TICKETING:
//TODO: юс╫ц
			if ( 0 < mOrder )
			{
				(*glpService).console( )->print( std::to_string(mOrder), sf::Color::Green );
			}
			[[ fallthrough ]];
		case State::SUBMITTING_TICKET:
			// "Connecting"
			//TODO
			//mSprite.setTextureRect( sf::IntRect( 0, 0, cast.x, cast.y ) );
			//mWindow_.draw( mSprite );
			break;
		default:
#ifdef _DEBUG
			__debugbreak( );
#else
			__assume(0);
#endif
	}
}

void scene::online::Waiting::cancelConnection( const std::string_view& )
{
	mHasCanceled = true;
}
