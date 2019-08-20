#include "Ready.h"
#include "../../ServiceLocatorMirror.h"
#include <Lib/ScriptLoader.h>
#include "Playing.h"

scene::inPlay::Ready::Ready( sf::RenderWindow& window, sf::Drawable& shapeOrSprite )
	: mFPS( 60u ), mFrameCount( mFPS * 3u ),
	mWindow( window ), mBackgroundRect( static_cast< sf::RectangleShape& >( shapeOrSprite ) ),
	mSpriteClipSize( 256u, 256u )
{
	auto& varT = ::ServiceLocatorMirror::Vault( );
	constexpr HashedKey HK_FORE_FPS = ::util::hash::Digest( "foreFPS" );
	if ( const auto it = varT.find( HK_FORE_FPS ); varT.cend( ) != it )
	{
		mFPS = it->second;
		mFrameCount = mFPS * 3u;
	}

	loadResources( );

	// Cyan
	const uint32_t BACKGROUND_RGB = 0x29cdb500u;
	const uint32_t FADE = 0x7fu;
	mBackgroundRect.setFillColor( sf::Color( BACKGROUND_RGB | FADE ) );
}

void scene::inPlay::Ready::loadResources( )
{
	const std::string scriptPathNName( "Scripts/Ready.lua" );
	const std::string varName0( "Sprite" );
	const std::string varName1( "SpriteClipWidth" );
	const std::string varName2( "SpriteClipHeight" );
	const auto result = ::util::script::LoadFromScript( scriptPathNName, varName0, varName1, varName2 );
	bool isDefault = false;
	// When there's the variable 'Sprite' in the script,
	if ( const auto it = result.find( varName0 ); result.cend( ) != it )
	{
		// Type check
		if ( true == std::holds_alternative< std::string >( it->second ) )
		{
			if ( false == mTexture.loadFromFile( std::get< std::string >( it->second ) ) )
			{
				// File Not Found Exception
				ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::FILE_NOT_FOUND, varName0, scriptPathNName );
			}
		}
		// Type Check Exception
		else
		{
			ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK, varName0, scriptPathNName );
		}
	}
	// Variable Not Found Exception
	else
	{
		ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::VARIABLE_NOT_FOUND, varName0, scriptPathNName );
	}

	if ( true == isDefault )
	{
		const std::string defaultFilePathNName( "Images/Ready.png" );
		if ( false == mTexture.loadFromFile( defaultFilePathNName ) )
		{
			// Exception: When there's not even the default file,
			ServiceLocatorMirror::Console( )->printFailure( FailureLevel::FATAL, "File Not Found: " + defaultFilePathNName );
#ifdef _DEBUG
			__debugbreak( );
#endif
		}
	}
	
	isDefault = true;
	// When there's the variable 'SpriteClipWidth' in the script,
	if ( const auto it = result.find( varName1 ); result.cend( ) != it )
	{
		// Type check
		if ( true == std::holds_alternative< int >( it->second ) )
		{
			int temp = std::get< int >( it->second );
			// Range Check Exception
			if ( 0 > temp )
			{
				ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::RANGE_CHECK, varName1, scriptPathNName );
			}
			// When the value looks OK,
			else
			{
				mSpriteClipSize.x = temp;
				isDefault = false;
			}
		}
		// Type Check Exception
		else
		{
			ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK, varName1, scriptPathNName );
		}
	}
	/// Variable Not Found Exception
	/// else { // Nothing to do }

	if ( true == isDefault )
	{
		ServiceLocatorMirror::Console( )->print( "Thus, width clipping the READY sprite is set 256." );
	}

	isDefault = true;
	// When there's the variable 'SpriteClipHeight' in the script,
	if ( const auto it = result.find( varName2 ); result.cend( ) != it )
	{
		// Type check
		if ( true == std::holds_alternative< int >( it->second ) )
		{
			int temp = std::get< int >( it->second );
			// Range Check Exception
			if ( 0 > temp )
			{
				ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::RANGE_CHECK, varName2, scriptPathNName );
			}
			// When the value looks OK,
			else
			{
				mSpriteClipSize.y = temp;
				isDefault = false;
			}
		}
		// Type Check Exception
		else
		{
			ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK, varName2, scriptPathNName );
		}
	}
	/// Variable Not Found Exception
	/// else { // Nothing to do }

	if ( true == isDefault )
	{
		ServiceLocatorMirror::Console( )->print( "Thus, height clipping the READY sprite is set 256." );
	}
	mSprite.setTexture( mTexture );
	mSprite.setPosition( sf::Vector2f( mWindow.getSize( ) - mSpriteClipSize ) * 0.5f );
}

void scene::inPlay::Ready::update( ::scene::inPlay::IScene** const nextScene )
{
	// NOTE: moved into 'draw( ).'
	///--mFrameCount;

	if ( 0 == mFrameCount )
	{
		*nextScene = new ::scene::inPlay::Playing( mWindow, mBackgroundRect );
	}
}

void scene::inPlay::Ready::draw( )
{
	////
	// Background
	////

	// Cyan
	const uint32_t BACKGROUND_RGB = 0x29cdb500u;
	const uint32_t FADE = 0x7fu;
	mBackgroundRect.setFillColor( sf::Color( BACKGROUND_RGB | FADE ) );
	mWindow.draw( mBackgroundRect );

	////
	// Countdown
	////
	mSprite.setTextureRect( sf::IntRect( 0, mSpriteClipSize.y*( mFrameCount/mFPS ), mSpriteClipSize.x, mSpriteClipSize.y ) );
	mWindow.draw( mSprite );

	--mFrameCount;
}
