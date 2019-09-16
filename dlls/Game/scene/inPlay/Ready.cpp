#include "Ready.h"
#include "../../ServiceLocatorMirror.h"
#include <Lib/ScriptLoader.h>
#include "Playing.h"

scene::inPlay::Ready::Ready( sf::RenderWindow& window, sf::Drawable& shapeOrSprite )
	: mFPS_( 60u ), mFrameCount( mFPS_ * 3 ),
	mWindow_( window ), mBackgroundRect_( static_cast< sf::RectangleShape& >( shapeOrSprite ) ),
	mSpriteClipSize_( 256u, 256u )
{
	auto& varT = ::ServiceLocatorMirror::Vault( );
	constexpr HashedKey HK_FORE_FPS = ::util::hash::Digest( "foreFPS" );
	if ( const auto it = varT.find( HK_FORE_FPS ); varT.cend( ) != it )
	{
		mFPS_ = it->second;
		mFrameCount = mFPS_ * 3;
	}

	loadResources( );

	// Cyan
	const uint32_t BACKGROUND_RGB = 0x29cdb500u;
	const uint32_t FADE = 0x7fu;
	mBackgroundRect_.setFillColor( sf::Color( BACKGROUND_RGB | FADE ) );
}

void scene::inPlay::Ready::loadResources( )
{
	const char scriptPathNName[] = "Scripts/Ready.lua"; //TODO: 테이블로 불러오기
	const char varName0[] = "Sprite";
	const char varName1[] = "SpriteClipWidth";
	const char varName2[] = "SpriteClipHeight";
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
		const char defaultFilePathNName[] = "Images/Ready.png";
		if ( false == mTexture.loadFromFile( defaultFilePathNName ) )
		{
			// Exception: When there's not even the default file,
			ServiceLocatorMirror::Console( )->printFailure( FailureLevel::FATAL, std::string("File Not Found: ")+defaultFilePathNName );
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
				mSpriteClipSize_.x = temp;
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
				mSpriteClipSize_.y = temp;
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
	mSprite.setPosition( sf::Vector2f( mWindow_.getSize( ) - mSpriteClipSize_ ) * 0.5f );
}

void scene::inPlay::Ready::update( ::scene::inPlay::IScene** const nextScene, std::queue< sf::Event >& eventQueue )
{
	// NOTE: moved into 'draw( ).'
	///--mFrameCount;

	if ( 0 == mFrameCount )
	{
		*nextScene = new ::scene::inPlay::Playing( mWindow_, mBackgroundRect_ );
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
	mBackgroundRect_.setFillColor( sf::Color( BACKGROUND_RGB | FADE ) );
	mWindow_.draw( mBackgroundRect_ );

	////
	// Countdown
	////
	mSprite.setTextureRect( sf::IntRect( 0, mSpriteClipSize_.y*( mFrameCount/mFPS_ ), mSpriteClipSize_.x, mSpriteClipSize_.y ) );
	mWindow_.draw( mSprite );

	--mFrameCount;
}
