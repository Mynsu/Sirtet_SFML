#include "Combo.h"
#include <thread>

void vfx::Combo::draw( const uint8_t lineCleared )
{
	ASSERT_TRUE( 0 < lineCleared );
	mSprite.setTextureRect( sf::IntRect( 0, (lineCleared-1)*mSpriteDimension.y, mSpriteDimension.x, mSpriteDimension.y) );
	//TODO: �����̴� ȿ��
	mWindow_.draw( mSprite );
}
