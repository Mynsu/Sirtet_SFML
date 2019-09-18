#include "Assertion.h"
#include "../../ServiceLocatorMirror.h"

scene::inPlay::Assertion::Assertion( sf::RenderWindow& window, bool* isESCPressed )
	: mFrameCount( 0u ), mIsESCPressed( isESCPressed ), mWindow( window )
{
	constexpr HashedKey HK_FORE_FPS = ::util::hash::Digest( "foreFPS", 7 );
	mFPS = static_cast<uint32_t>(::ServiceLocatorMirror::Vault( )[ HK_FORE_FPS ]);

	*mIsESCPressed = true;

	mRect.setSize( sf::Vector2f( window.getSize( ) ) );
	const uint8_t alpha = 0x7f;
	mRect.setFillColor( sf::Color( alpha ) );
}

void scene::inPlay::Assertion::loadResources( )
{
}

int8_t scene::inPlay::Assertion::update( ::scene::inPlay::IScene** const, std::list<sf::Event>& eventQueue )
{
	int8_t retVal = 0;

	// 2 seconds after created,
	if ( 2*mFPS == mFrameCount )
	{
		*mIsESCPressed = false;
		retVal = -1;
	}
	else
	{
		for ( auto it = eventQueue.cbegin(); eventQueue.cend() != it; ++it )
		{
			if ( sf::Event::KeyPressed == it->type && sf::Keyboard::Escape == it->key.code )
			{
				retVal = 1;
				eventQueue.erase( it );
				break;
			}
		}
	}

	return retVal;
}

void scene::inPlay::Assertion::draw( )
{
	mWindow.draw( mRect );
	++mFrameCount;
}