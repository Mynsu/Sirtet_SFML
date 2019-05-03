#include "MainMenu.h"

namespace sequence
{
	MainMenu::MainMenu( sf::RenderWindow& window )
		: mWindow( window )
	{
		const std::string pathAndFilename( "Images/MainMenu.png" );
		if ( false == mTexture.loadFromFile( pathAndFilename ) )
		{
			Console_->printError( "Failed to load " + pathAndFilename );
		}
		mSprite.setTexture( mTexture );
	}

	void MainMenu::update( )
	{

	}

	void MainMenu::draw( )
	{
		mWindow.draw( mSprite );
	}
}
