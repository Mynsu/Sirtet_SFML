#include "Opening.h"

namespace sequence
{
	Opening::Opening( sf::RenderWindow& window )
		: mWindow( window )
	{
		const std::string pathAndFilename( "Images/Opening.png" );
		if ( false == mTexture.loadFromFile( pathAndFilename, sf::IntRect( 0u, 100u, 800u, 600u ) ) )
		{
			Console_->printError( "Failed to load " + pathAndFilename );
		}
		mSprite.setTexture( mTexture );
	}

	void Opening::update( )
	{
		static int frameCount = 0;//
		++frameCount;

		if ( 60 * 3 < frameCount )//TODO
		{
			
		}
	}

	void Opening::draw( )
	{
		mWindow.draw( mSprite );
	}
}