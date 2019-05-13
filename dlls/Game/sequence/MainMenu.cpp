#include "MainMenu.h"

namespace sequence
{
	bool MainMenu::IsInstanciated = false;

	MainMenu::MainMenu( sf::RenderWindow& window,
						::sequence::Seq* const nextSequence )
		: mWindow( window ),
		mNextSequence( nextSequence )
	{
		ASSERT_FALSE( IsInstanciated );
		const std::string pathAndFilename( "Images/MainMenu.png" );
		if ( false == mTexture.loadFromFile( pathAndFilename ) )
		{
			global::Console( )->printError( "Failed to load " + pathAndFilename );
		}
		mSprite.setTexture( mTexture );
		*mNextSequence = ::sequence::Seq::NONE;
		IsInstanciated = true;
	}

	void MainMenu::update( )
	{
		if ( true == sf::Mouse::isButtonPressed( sf::Mouse::Left ) )
		{
			*mNextSequence = ::sequence::Seq::SINGLE_PLAY;
		}
	}

	void MainMenu::draw( )
	{
		mSprite.setTextureRect( sf::IntRect( 0u, 0u, 286u, 120u ) );
		mSprite.setPosition( sf::Vector2f( 480.f, 400.f ) );
		mWindow.draw( mSprite );

		const int32_t buttonPositionX = 250;
		const int32_t buttonPositionY = 100;
		const int32_t buttonWidth = 286;
		const int32_t buttonHeight = 120;
		mSprite.setPosition( sf::Vector2f( sf::Vector2i( buttonPositionX, buttonPositionY ) ) );
		const sf::Vector2i mousePosition( sf::Mouse::getPosition( mWindow ) );
		if ( buttonPositionX <= mousePosition.x && mousePosition.x <= buttonPositionX + buttonWidth
			 && buttonPositionY <= mousePosition.y && mousePosition.y <= buttonPositionY + buttonHeight )
		{
			mSprite.setTextureRect( sf::IntRect( 0, 225, buttonWidth, buttonHeight ) );
		}
		else
		{
			mSprite.setTextureRect( sf::IntRect( 0, 120, buttonWidth, buttonHeight ) );
		}
		mWindow.draw( mSprite );
	}
}
