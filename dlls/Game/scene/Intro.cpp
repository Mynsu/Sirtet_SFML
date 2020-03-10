#include "../pch.h"
#include "Intro.h"
#include <Lib/ScriptLoader.h>
#include "../ServiceLocatorMirror.h"

bool ::scene::Intro::IsInstantiated = false;

::scene::Intro::Intro( const sf::RenderWindow& window )
	: mDuration( 2 ),
	mAlpha( 0x00 ),
	mFrameCountToStart( 0 ), mFPS_( 60 ),
	mNextScene( ::scene::ID::MAIN_MENU )
{
	ASSERT_TRUE( false == IsInstantiated );

	gService()->sound().stopBGM();
	if ( const auto it = gService()->vault().find(HK_FORE_FPS); gService()->vault().cend() != it )
	{
		mFPS_ = (uint16_t)it->second;
	}
	loadResources( window );

	IsInstantiated = true;
}

::scene::Intro::~Intro( )
{
	IsInstantiated = false;
}

void scene::Intro::loadResources( const sf::RenderWindow& window )
{
	std::string scriptPath( "Scripts/Intro.lua" );
	std::string varName0( "Image" );
	std::string introImagePath( "Images/Intro.png"	);
	std::string varName1( "NextScene" );
	const auto result = ::util::script::LoadFromScript(scriptPath, varName0, varName1);
	if ( const auto it = result.find(varName0);
		result.cend( ) != it )
	{
		// Type check
		if ( true == std::holds_alternative<std::string>(it->second) )
		{
			introImagePath = std::get<std::string>(it->second);
		}
		else
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
												   varName0, scriptPath );
		}
	}
	else
	{
		gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
											   varName0, scriptPath );
	}

	if ( false == mTexture.loadFromFile(introImagePath) )
	{
		gService()->console().printFailure( FailureLevel::WARNING,
										   "File Not Found: "+introImagePath );
	}
	mSprite.setTexture( mTexture );
	const sf::Vector2u winSize( window.getSize() );
	mSprite.setTextureRect( sf::IntRect(0, 0, winSize.x, winSize.y) );

	if ( const auto it = result.find(varName1);
		result.cend( ) != it )
	{
		// Type check whether it can be cast to enum type or not,
		if ( true == std::holds_alternative<int>(it->second) )
		{
			mNextScene = (::scene::ID)std::get<int>(it->second);
		}
		else
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK, varName1, scriptPath );
		}
	}
	else
	{
		gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND, varName1, scriptPath );
	}
}

::scene::ID scene::Intro::update( std::vector<sf::Event>& )
{
	if ( (uint16_t)mFPS_*mDuration < mFrameCountToStart )
	{
		return mNextScene ;
	}

	return ::scene::ID::AS_IS;
}

void ::scene::Intro::draw( sf::RenderWindow& window )
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
	window.draw( mSprite );

	++mFrameCountToStart;
}

::scene::ID scene::Intro::currentScene( ) const
{
	return ::scene::ID::INTRO;
}
