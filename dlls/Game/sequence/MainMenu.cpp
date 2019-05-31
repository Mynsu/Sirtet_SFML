#include "MainMenu.h"

namespace sequence
{
	bool MainMenu::IsInstanciated = false;

	MainMenu::MainMenu( sf::RenderWindow& window,
						::sequence::Seq* const nextMainSequence )
		: ::sequence::ISequence( window, nextMainSequence ),
		mOnIndicator( ::sequence::Seq::NONE )
	{
		ASSERT_FALSE( IsInstanciated );
		const std::string pathAndFilename( "Images/MainMenu.png" );//TODO: 변수테이블, config 파일에서 불러오기
		if ( false == iTexture.loadFromFile( pathAndFilename ) )
		{
			global::Console( )->printError( "Failed to load " + pathAndFilename );
		}
		iSprite.setTexture( iTexture );
		*iNextMainSequence = ::sequence::Seq::NONE;
		IsInstanciated = true;
	}

	void MainMenu::update( )
	{
		if ( true == sf::Mouse::isButtonPressed( sf::Mouse::Left ) )
		{
			switch ( mOnIndicator )
			{
				case ::sequence::Seq::SINGLE_PLAY:
					*iNextMainSequence = mOnIndicator;
					break;
				default:
					break;
			}
		}
	}

	void MainMenu::draw( )
	{
		const sf::Vector2f winSize( iWindow.getSize( ) );
		const sf::Vector2i clipSize( iTexture.getSize( ).x, 120 );//TODO: 파일 읽어서 정하자
		const sf::Vector2f logoMargin( 80.f, 80.f );
		iSprite.setPosition( winSize - sf::Vector2f( clipSize ) - logoMargin );
		iSprite.setTextureRect( sf::IntRect( 0, 0, clipSize.x, clipSize.y ) );
		iWindow.draw( iSprite );

		iSprite.setPosition( winSize / 3.2f );
		if ( true == iSprite.getGlobalBounds( ).contains( sf::Vector2f( sf::Mouse::getPosition( iWindow ) ) ) )
		{
			mOnIndicator = ::sequence::Seq::SINGLE_PLAY;
			iSprite.setTextureRect( sf::IntRect( 0, 225, clipSize.x, clipSize.y ) );
		}
		else
		{
			mOnIndicator = ::sequence::Seq::NONE;
			iSprite.setTextureRect( sf::IntRect( 0, 120, clipSize.x, clipSize.y ) );
		}
		iWindow.draw( iSprite );
	}
}
