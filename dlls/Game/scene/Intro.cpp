#include "Intro.h"
#include <Lib/ScriptLoader.h>
#include "../ServiceLocatorMirror.h"

bool ::scene::Intro::IsInstantiated = false;

::scene::Intro::Intro( sf::RenderWindow& window, const SetScene_t& setScene )
	: mDuration( 2u ),
	mAlpha( 0x00u ),
	mFrameCount( 0u ),
	mFPS( 60u ),
	mWindow( window ),
	mSetScene( setScene ),
	mNextScene( ::scene::ID::MAIN_MENU )
{
	ASSERT_FALSE( IsInstantiated );

	constexpr HashedKey HK_FORE_FPS = ::util::hash::Digest( "foreFPS" );
	if ( const auto it = ServiceLocatorMirror::Vault( ).find( HK_FORE_FPS ); ServiceLocatorMirror::Vault( ).cend( ) != it )
	{
		mFPS = static_cast< uint16_t >( it->second );
	}

	loadResources( );

	IsInstantiated = true;
}

::scene::Intro::~Intro( )
{
	IsInstantiated = false;
}

void scene::Intro::loadResources( )
{
	const std::string scriptPathNName( "Scripts/Intro.lua" );
	const std::string varName0( "Image" );
	const std::string varName1( "NextScene" );
	const auto result = ::util::script::LoadFromScript( scriptPathNName, varName0, varName1 );
	bool isDefault = false;
	// When there's the variable 'Image' in the script,
	if ( const auto it = result.find( varName0 ); result.cend( ) != it )
	{
		// Type check
		if ( true == std::holds_alternative< std::string >( it->second ) )
		{
			if ( false == mTexture.loadFromFile( std::get< std::string >( it->second ) ) )
			{
				// File Not Found Exception
				ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::FILE_NOT_FOUND, varName0, scriptPathNName );
				isDefault = true;
			}
		}
		// Type Check Exception
		else
		{
			ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK, varName0, scriptPathNName );
			isDefault = true;
		}
	}
	// Variable Not Found Exception
	else
	{
		ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::VARIABLE_NOT_FOUND, varName0, scriptPathNName );
		isDefault = true;
	}

	if ( true == isDefault )
	{
		const std::string defaultFilePathNName( "Images/Intro.png" );
		if ( false == mTexture.loadFromFile( defaultFilePathNName ) )
		{
			// Exception: When there's not even the default file,
			ServiceLocatorMirror::Console( )->printFailure( FailureLevel::FATAL, "File Not Found: " + defaultFilePathNName );
#ifdef _DEBUG
			__debugbreak( );
#endif
		}
	}
	mSprite.setTexture( mTexture );
	const sf::Vector2< int16_t > winSize( mWindow.getSize( ) );
	mSprite.setTextureRect( sf::IntRect( 0, 0, winSize.x, winSize.y ) );
	// NOTE: Setting scale makes an image distorted and a bunch of bricks.
	///mSprite.scale( scaleWidthRatio, scaleWidthRatio );

	isDefault = false;
	// When there's the variable 'NextScene' in the script,
	if ( const auto it = result.find( varName1 ); result.cend( ) != it )
	{
		// Type check whether it can be cast to enum type or not,
		if ( true == std::holds_alternative< int >( it->second ) )
		{
			// Range check
			if ( const ::scene::ID val = static_cast< ::scene::ID >( std::get< int >( it->second ) );
				 val < ::scene::ID::MAX_NONE )
			{
				mNextScene = val;
			}
			// Range Check Exception
			else
			{
				ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::RANGE_CHECK, varName1, scriptPathNName );
				isDefault = true;
			}
		}
		// Type Check Exception
		else
		{
			ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK, varName1, scriptPathNName );
			isDefault = true;
		}
	}
	// Variable Not Found Exception
	else
	{
		ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::VARIABLE_NOT_FOUND, varName1, scriptPathNName );
		isDefault = true;
	}

	if ( true == isDefault )
	{
		ServiceLocatorMirror::Console( )->print( "Thus, scene MAIN_MENU appears after scene INTRO." );
	}
}

void ::scene::Intro::update( std::queue< sf::Event >& )
{
	//
	// Scene Transition
	//
	if ( mFPS * mDuration < mFrameCount )
	{
		mSetScene( mNextScene );
	}

	// NOTE: Moved into draw( ).
	///++mFrameCount;
}

void ::scene::Intro::draw( )
{
	//
	// Fade In & Out
	//
	const uint8_t MAX_RGBA = 0xffu;
	const uint8_t MIN_RGBA = 0x00u;
	// diff accumulates 0.5 second, or 500 milliseconds after.
	const uint8_t diff = static_cast< uint8_t >( MAX_RGBA / ( mFPS * 0.5f ) );
	const uint16_t brokenPoint = mFPS * mDuration - 30u;
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

::scene::ID scene::Intro::currentScene( ) const
{
	return ::scene::ID::INTRO;
}

//auto ::scene::Intro::newEqualTypeInstance( ) -> std::unique_ptr< ::scene::IScene >
//{
//	IsInstantiated = false;
//	return std::make_unique< ::scene::Intro >( mWindow, mSetScene );
//}