#include "../../pch.h"
#include "Assertion.h"
#include "../../ServiceLocatorMirror.h"
#include "../VaultKeyList.h"

scene::inPlay::Assertion::Assertion( sf::RenderWindow& window )
	: mFrameCount( 0u ), mWindow_( window )
{
	mFPS_ = static_cast< uint32_t >( (*glpService).vault()[HK_FORE_FPS] );

	mRect.setSize( sf::Vector2f( window.getSize( ) ) );
	const uint8_t alpha = 0x7f;
	mRect.setFillColor( sf::Color( alpha ) );
}

void scene::inPlay::Assertion::loadResources( )
{
}

::scene::inPlay::ID scene::inPlay::Assertion::update( std::list<sf::Event>& eventQueue )
{
	::scene::inPlay::ID retVal = ::scene::inPlay::ID::AS_IS;
	// 2 seconds after created,
	if ( 2*mFPS_ == mFrameCount )
	{
		retVal = ::scene::inPlay::ID::OFF;
	}
	else
	{
		for ( auto it = eventQueue.cbegin(); eventQueue.cend() != it; ++it )
		{
			if ( sf::Event::KeyPressed == it->type && sf::Keyboard::Escape == it->key.code )
			{
				it = eventQueue.erase( it );
				retVal = ::scene::inPlay::ID::MAIN_MENU;
			}
		}
	}

	return retVal;
}

void scene::inPlay::Assertion::draw( )
{
	mWindow_.draw( mRect );
	++mFrameCount;
}