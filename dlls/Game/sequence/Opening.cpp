#include "Opening.h"

namespace sequence
{
	Opening::Opening( const sf::RenderWindow& window )
		: mWindow( window )
	{
		if ( false == mTexture.loadFromFile( "../Image/Opening.png", sf::IntRect( 0u, 100u, 800u, 600u ) ) )
		{

		}
	}

	void Opening::update( )
	{
	}

	void Opening::draw( )
	{
	}
}