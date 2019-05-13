#include "Opening.h"

namespace sequence
{
	bool Opening::IsInstanciated = false;

	Opening::Opening( sf::RenderWindow& window
					  , ::sequence::Seq* nextSequence )
		: mDuration( 2u ),
		mAlpha( 0u ),
		mFrameCount( 0u ),
		mWindow( window ),
		mNextSequence( nextSequence )
	{
		ASSERT_FALSE( IsInstanciated );
		*mNextSequence = ::sequence::Seq::NONE;
		const std::string pathAndFilename( "Images/Opening.png" );
		if ( false == mTexture.loadFromFile( pathAndFilename, sf::IntRect( 0u, 100u, 800u, 600u ) ) )
		{
			global::Console( )->printError( "Failed to load " + pathAndFilename );
		}
		mSprite.setTexture( mTexture );
		IsInstanciated = true;
	}

	void Opening::update( )
	{
		//
		// Sequence Transition
		//
		// FPS change promptly permeates at the next frame, not after a new instance comes.
		const uint8_t fps = static_cast< uint8_t >( global::VariableTable( ).find( 863391493 )->second );
		if ( fps * mDuration < mFrameCount )
		{
			*mNextSequence = ::sequence::Seq::MAIN_MENU;
		}
	}

	void Opening::draw( )
	{
		//
		// Fade In & Out
		//
		const uint8_t MAX_RGBA = 0xffu;
		const uint8_t MIN_RGBA = 0x00u;
		// FPS change promptly permeates at the next frame, not after a new instance comes.
		const uint8_t fps = static_cast< uint8_t >( global::VariableTable( ).find( 863391493 )->second );
		const uint8_t diff = MAX_RGBA / ( fps / 2u );
		const uint16_t brokenPoint = fps * mDuration - 30u;
		if ( mFrameCount > brokenPoint )
		{
			// NOTE: Both 'mAlpha' and 'diff' are 'uint8_t', but 'mAlpha - diff' is 'int', not 'int8_t',
			// that means 'mAlpha - diff' can be below zero, no underflow happens.
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
		mSprite.setColor( sf::Color( MAX_RGBA, MAX_RGBA, MAX_RGBA, mAlpha ) );
		mWindow.draw( mSprite );
		++mFrameCount;
	}
}