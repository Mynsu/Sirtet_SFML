#include "../pch.h"
#include "Intro.h"
#include <Lib/ScriptLoader.h>
#include <Lib/VaultKeyList.h>
#include "../ServiceLocatorMirror.h"

bool ::scene::Intro::IsInstantiated = false;

::scene::Intro::Intro( sf::RenderWindow& window )
	: mDuration( 2 ),
	mAlpha_( 0x00 ),
	mFrameCountToStart( 0 ),	mFPS_( 60 ),
	mWindow_( window ),
	mNextScene_( ::scene::ID::MAIN_MENU )
{
	ASSERT_TRUE( false == IsInstantiated );

	if ( const auto it = gService()->vault().find(HK_FORE_FPS); gService()->vault().cend() != it )
	{
		mFPS_ = (uint16_t)it->second;
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
	std::string scriptPathNName( "Scripts/Intro.lua" );
	std::string varName0( "Image" );
	std::string introImagePathNName( "Images/Intro.png"	);
	std::string varName1( "NextScene" );
	const auto result = ::util::script::LoadFromScript(scriptPathNName, varName0, varName1);
	if ( const auto it = result.find(varName0);
		result.cend( ) != it )
	{
		// Type check
		if ( true == std::holds_alternative<std::string>(it->second) )
		{
			introImagePathNName = std::get<std::string>(it->second);
		}
		// Type Check Exception
		else
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK, varName0, scriptPathNName );
		}
	}
	// Variable Not Found Exception
	else
	{
		gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND, varName0, scriptPathNName );
	}

	if ( false == mTexture.loadFromFile(introImagePathNName) )
	{
		// Exception: When there's not even the default file,
		gService()->console().printFailure( FailureLevel::FATAL, "File Not Found: "+introImagePathNName );
	}
	mSprite.setTexture( mTexture );
	const sf::Vector2u winSize( mWindow_.getSize() );
	mSprite.setTextureRect( sf::IntRect(0, 0, winSize.x, winSize.y) );

	if ( const auto it = result.find(varName1);
		result.cend( ) != it )
	{
		// Type check whether it can be cast to enum type or not,
		if ( true == std::holds_alternative<int>(it->second) )
		{
			// Range check
			if ( const ::scene::ID val = (::scene::ID)std::get<int>(it->second);
				 val < ::scene::ID::MAX )
			{
				mNextScene_ = val;
			}
			// Range Check Exception
			else
			{
				gService()->console().printScriptError( ExceptionType::RANGE_CHECK, varName1, scriptPathNName );
			}
		}
		// Type Check Exception
		else
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK, varName1, scriptPathNName );
		}
	}
	// Variable Not Found Exception
	else
	{
		gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND, varName1, scriptPathNName );
	}
}

::scene::ID scene::Intro::update( std::vector<sf::Event>& )
{
	if ( (uint16_t)mFPS_*mDuration < mFrameCountToStart )
	{
		return mNextScene_ ;
	}

	return ::scene::ID::AS_IS;
}

void ::scene::Intro::draw( )
{
	//
	// Fade In & Out
	//
	const uint8_t MAX_RGBA = 0xff;
	const uint8_t MIN_RGBA = 0x0;
	// diff accumulates 0.5 second, or 500 milliseconds after.
	const uint8_t diff = (uint8_t)(MAX_RGBA / (mFPS_*0.5f));
	const uint16_t brokenPoint = (uint16_t)mFPS_*mDuration/2;
	if ( mFrameCountToStart > brokenPoint )
	{
		// NOTE: Both 'mAlpha' and 'diff' are 'uint8_t', but 'mAlpha - diff' is 'int', not 'int8_t.'
		// That means 'mAlpha - diff' can be below zero, so no underflow happens.
		if ( mAlpha_ - diff < MIN_RGBA )
		{
			mAlpha_ = MIN_RGBA;
		}
		// Fade Out
		else
		{
			mAlpha_ -= diff;
		}
	}
	else
	{
		// NOTE: Same as above.
		if ( mAlpha_ + diff > MAX_RGBA )
		{
			mAlpha_ = MAX_RGBA;
		}
		// Fade In
		else
		{
			mAlpha_ += diff;
		}
	}
	mSprite.setColor( sf::Color( MAX_RGBA, MAX_RGBA, MAX_RGBA, mAlpha_ ) );
	mWindow_.draw( mSprite );

	++mFrameCountToStart;
}

#ifdef _DEV
::scene::ID scene::Intro::currentScene( ) const
{
	return ::scene::ID::INTRO;
}
#endif