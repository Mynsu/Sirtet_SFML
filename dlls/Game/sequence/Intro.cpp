#include "Intro.h"

namespace sequence
{
	bool Intro::IsInstanciated = false;

	Intro::Intro( sf::RenderWindow& window
					  , ::sequence::Seq* const nextMainSequence )
		: ::sequence::ISequence( window, nextMainSequence ),
		mDuration( 2u ),
		mAlpha( 0u ),
		mFrameCount( 0u )
	{
		ASSERT_FALSE( IsInstanciated );
		*iNextMainSequence = ::sequence::Seq::NONE;
		const std::string pathAndFilename( "Images/Intro.png" );//TODO: 변수테이블, config 파일에서 불러오기
		if ( false == iTexture.loadFromFile( pathAndFilename ) )///, sf::IntRect( 0u, 100u, 800u, 600u ) ) )
		{
			global::Console( )->printError( "Failed to load " + pathAndFilename );
		}
		iSprite.setTexture( iTexture );
		const sf::Vector2i textureSize( iTexture.getSize( ) );
		const sf::Vector2i winSize( window.getSize( ) );
		iSprite.setTextureRect( sf::IntRect( ( textureSize - winSize ) / 2, winSize ) );
		IsInstanciated = true;
	}

	void Intro::update( )
	{
		//
		// Sequence Transition
		//
		// FPS change promptly permeates at the next frame, not after a new instance comes.
		const uint8_t fps = static_cast< uint8_t >( global::VariableTable( ).find( 1189622459 )->second ); // foreFPS
		if ( fps * mDuration < mFrameCount )
		{
			*iNextMainSequence = ::sequence::Seq::MAIN_MENU;
		}
	}

	void Intro::draw( )
	{
		//
		// Fade In & Out
		//
		const uint8_t MAX_RGBA = 0xffu;
		const uint8_t MIN_RGBA = 0x00u;
		// FPS change promptly permeates at the next frame, not after a new instance comes.
		const uint8_t fps = static_cast< uint8_t >( global::VariableTable( ).find( 1189622459 )->second ); // foreFPS
		const uint8_t diff = MAX_RGBA / ( fps / 2u );
		const uint16_t brokenPoint = fps * mDuration - 30u;
		if ( mFrameCount > brokenPoint )
		{
			// NOTE: Both 'mAlpha' and 'diff' are 'uint8_t', but 'mAlpha - diff' is 'int', not 'int8_t.'
			// That means 'mAlpha - diff' can be below zero, so no underflow happens.
			if ( mAlpha - diff < MIN_RGBA )
			{
				mAlpha = MIN_RGBA;
			}
			// Fade Out
			else
			{
				mAlpha -= diff;
			}
		}
		else
		{
			// NOTE: Same as above.
			if ( mAlpha + diff > MAX_RGBA )
			{
				mAlpha = MAX_RGBA;
			}
			// Fade In
			else
			{
				mAlpha += diff;
			}
		}
		iSprite.setColor( sf::Color( MAX_RGBA, MAX_RGBA, MAX_RGBA, mAlpha ) );
		iWindow.draw( iSprite );
		++mFrameCount;
	}
}