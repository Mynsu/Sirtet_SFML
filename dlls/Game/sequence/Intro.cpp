#include "Intro.h"
#include <Lib/ScriptLoader.h>

namespace sequence
{
	bool Intro::IsInstanciated = false;

	Intro::Intro( sf::RenderWindow& window, const std::function< void( const ::sequence::Seq ) >&& setSequence )
		: mDuration( 2u ),
		mAlpha( 0x00u ),
		mFrameCount( 0u ),
		mFPS( 60u ),
		mWindow( window ),
		mSetSequence( std::forward< const std::function< void( const ::sequence::Seq ) >&& >( setSequence ) ),
		mNextSequence( ::sequence::Seq::MAIN_MENU )
	{
		ASSERT_FALSE( IsInstanciated );

		constexpr HashedKey HK_FORE_FPS = ::util::hash::Digest( "foreFPS" );
		if ( const auto it = ServiceLocatorMirror::Vault( ).find( HK_FORE_FPS ); ServiceLocatorMirror::Vault( ).cend( ) != it )
		{
			mFPS = static_cast< uint16_t >( it->second );
		}

		const std::string scriptPathNName( "Scripts/Intro.lua" );
		const std::string varName0( "Image" );
		const std::string varName1( "NextSequence" );
		const auto result = ::util::script::LoadFromScript( scriptPathNName, varName0, varName1 );
		// When the variable 'Image' exists in the script,
		if ( const auto it = result.find( varName0 ); result.cend( ) != it )
		{
			// When its type is string,
			if ( true == std::holds_alternative< std::string >( it->second ) )
			{
				if ( false == mTexture.loadFromFile( std::get< std::string >( it->second ) ) )
				{
					// Exception: When it tells an odd path or there's no such file,
					ServiceLocatorMirror::Console( )->printFailure( FailureLevel::CRITICAL, "File not found: " + varName0 + " in " + scriptPathNName );
#ifdef _DEBUG
					__debugbreak( );
#endif
				}
			}
			// Exception: When its type isN'T string,
			else
			{
				ServiceLocatorMirror::Console( )->printScriptError( FailureLevel::WARNING, varName0, scriptPathNName );
			}
		}
		// Exception: When the variable 'Image' doesN'T exist in the script,
		else
		{
			ServiceLocatorMirror::Console( )->printScriptError( FailureLevel::WARNING, varName0, scriptPathNName );
			const std::string defaultFilePathNName( "Images/Intro.png" );
			if ( false == mTexture.loadFromFile( defaultFilePathNName ) )
			{
				// Exception: When there isn't the default file,
				ServiceLocatorMirror::Console( )->printFailure( FailureLevel::CRITICAL, "File not found: " + defaultFilePathNName );
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

		bool isScriptError = false;
		// When the variable 'NextSequence' exists in the script,
		if ( const auto it = result.find( varName1 ); result.cend( ) != it )
		{
			// When its type is integer, which can be cast to enum type,
			if ( true == std::holds_alternative< int >( it->second ) )
			{
				// Range check
				if ( const ::sequence::Seq val = static_cast< ::sequence::Seq >( std::get< int >( it->second ) );
					 val < ::sequence::Seq::MAX )
				{
					mNextSequence = val;
				}
				else
				{
					isScriptError = true;
				}
			}
			// Exception: When its type isN'T integer,
			else
			{
				isScriptError = true;
			}
		}
		// Exception: When the variable 'NextSequence' doesN'T exist in the script,
		else
		{
			isScriptError = true;
		}
		if ( true == isScriptError )
		{
			ServiceLocatorMirror::Console( )->printScriptError( FailureLevel::WARNING, varName1, scriptPathNName );
		}

		IsInstanciated = true;
	}

	void Intro::update( )
	{
		//
		// Sequence Transition
		//
		if ( mFPS * mDuration < mFrameCount )
		{
			mSetSequence( mNextSequence );
		}

		// NOTE: Moved into draw( ).
		///++mFrameCount;
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
		const uint16_t fps = 60u;//static_cast< uint16_t >( ::global::Vault( ).find( HK_FORE_FPS )->second );
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

	/*auto Intro::newInstanceOfEqualType( ) -> std::unique_ptr<ISequence>&&
	{
		return std::make_unique<::sequence::inPlay::Playing>(mWindow);
	}*/
}