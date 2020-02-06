#include "../pch.h"
#include "Combo.h"

vfx::Combo::Combo( sf::RenderWindow& window )
	: mWindow_( window )
{
}

void vfx::Combo::draw( const uint8_t lineCleared )
{
	ASSERT_TRUE( 0 < lineCleared );
	mSprite.setTextureRect( sf::IntRect( 0, (lineCleared-1)*mSpriteDimension.y, mSpriteDimension.x, mSpriteDimension.y) );
	mWindow_.draw( mSprite );
}
