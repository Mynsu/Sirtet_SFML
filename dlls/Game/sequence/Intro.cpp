#include "Intro.h"

namespace sequence
{
	bool Intro::IsInstanciated = false;

	Intro::Intro( sf::RenderWindow& window, ::sequence::Seq* const nextMainSequence )
		: mNextMainSequence( nextMainSequence ),
		mWindow( window ),
		mDuration( 2u ),
		mAlpha( 0u ),
		mFrameCount( 0u )
	{
		ASSERT_FALSE( IsInstanciated );
		// Like setting car's gear neutral
		*mNextMainSequence = ::sequence::Seq::NONE;
		const std::string scriptPathNName( "Scripts/Intro.lua" );
		const std::string varName( "Image" );
		const auto table = ::ServiceLocator::LoadFromScript( scriptPathNName, varName );
		// When the variable 'Image' exists in the script,
		if ( const auto& it = table.find( varName ); table.cend( ) != it )
		{
			if ( false == mTexture.loadFromFile( std::get< std::string >( it->second ) ) )
			{
				// When the value has an odd path, or there's no such file,
				::global::Console( )->printError( ErrorLevel::CRITICAL, "File not found: " + varName + " in " + scriptPathNName );
#ifdef _DEBUG
				__debugbreak( );
#endif
			}
		}
		// When the variable 'Image' doesN'T exist in the script,
		else
		{
			::global::Console( )->printScriptError( ErrorLevel::WARNING, varName, scriptPathNName );
			const std::string defaultFilePathNName( "Images/Intro.png" );
			if ( false == mTexture.loadFromFile( defaultFilePathNName ) )
			{
				// When there isn't the default file,
				::global::Console( )->printError( ErrorLevel::CRITICAL, "File not found: " + defaultFilePathNName );
#ifdef _DEBUG
				__debugbreak( );
#endif
			}
		}
		mSprite.setTexture( mTexture );
		const sf::Vector2i winSize( mWindow.getSize( ) );
		mSprite.setTextureRect( sf::IntRect( 0, 0, winSize.x, winSize.y ) );
		// NOTE: Setting scale makes an image distorted and a bunch of bricks.
		///mSprite.scale( scaleWidthRatio, scaleWidthRatio );
		IsInstanciated = true;
	}

	void Intro::update( )
	{
		//
		// Sequence Transition
		//
		// FPS change promptly permeates at the next frame, not after a new instance comes.
		constexpr HashedKey key = ::util::hash::Digest( "foreFPS" );
		const uint16_t fps = static_cast< uint16_t >( ::global::VariableTable( ).find( key )->second );
		if ( fps * mDuration < mFrameCount )
		{
			*mNextMainSequence = ::sequence::Seq::MAIN_MENU;
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
		constexpr HashedKey key = ::util::hash::Digest( "foreFPS" );
		const uint16_t fps = static_cast< uint16_t >( ::global::VariableTable( ).find( key )->second );
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
		mSprite.setColor( sf::Color( MAX_RGBA, MAX_RGBA, MAX_RGBA, mAlpha ) );
		mWindow.draw( mSprite );
		++mFrameCount;
	}
}