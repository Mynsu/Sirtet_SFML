#include "MainMenu.h"

namespace sequence
{
	MainMenu::MainMenu( sf::RenderWindow& window, ::sequence::Seq* const nextSequence )
		: mWindow( window ), mNextSequence( nextSequence )
	{
		const std::string pathAndFilename( "Images/MainMenu.png" );
		if ( false == mTexture.loadFromFile( pathAndFilename ) )
		{
			Console_->printError( "Failed to load " + pathAndFilename );
		}
		mSprite.setTexture( mTexture );
		*mNextSequence = ::sequence::Seq::NONE;
	}

	void MainMenu::update( )
	{

	}

	void MainMenu::draw( )
	{
		mWindow.draw( mSprite );
	}
}
