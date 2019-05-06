#include "Opening.h"

namespace sequence
{
	Opening::Opening( sf::RenderWindow& window
					  , ::sequence::Seq* nextSequence )
		: mDuration( 2u ),
		//mFPS( static_cast< uint8_t >( global::VariableTable( ).find( 863391493 )->second ) ),
		mFPS( 60u ),
		mWindow( window ),
		mNextSequence( nextSequence )
	{
		*mNextSequence = ::sequence::Seq::NONE;
		const std::string pathAndFilename( "Images/Opening.png" );
		if ( false == mTexture.loadFromFile( pathAndFilename, sf::IntRect( 0u, 100u, 800u, 600u ) ) )
		{
			//Console_->printError( "Failed to load " + pathAndFilename );
			global::Console( )->printError( "Failed to load " + pathAndFilename );
		}
		mSprite.setTexture( mTexture );
	}

	void Opening::update( )
	{
		//
		// Sequence Transition
		//
		static int frameCount = 0;// 궁금함: 이 변수의 위치는 함수 안?  클래스 멤버변수 옆?  어셈블리로 보자.
		++frameCount;
		if ( mFPS * mDuration < frameCount )//TODO
		{
			*mNextSequence = ::sequence::Seq::MAIN_MENU;
			frameCount = 0;
		}
	}

	void Opening::draw( )
	{
		//
		// Fade In & Out
		//
		// IMPORTANT: Overflow intended.
		static uint8_t frameCount = 0;
		frameCount += 255u / ( mFPS * mDuration );//TODO
		mSprite.setColor( sf::Color( 255u, 255u, 255u, frameCount ) );
		mWindow.draw( mSprite );
	}
}