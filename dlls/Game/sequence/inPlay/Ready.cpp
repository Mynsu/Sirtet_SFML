#include "Ready.h"
#include "../../Common.h"

sequence::inPlay::Ready::Ready( sf::RenderWindow& window,
								::sequence::inPlay::Seq* const nextInPlaySequence,
								sf::Drawable& shapeOrSprite )
	: mFrameRate( 60 ), mCountDown( 3*mFrameRate ),
	mWindow( window ), mBackgroundRect( static_cast< sf::RectangleShape& >( shapeOrSprite ) ),
	mNextInPlaySequence( nextInPlaySequence ),
	mSpriteClipSize( 256, 256 )
{
	*mNextInPlaySequence = ::sequence::inPlay::Seq::NONE;
	auto& varT = ::global::VariableTable( );
	constexpr HashedKey key = ::util::hash::Digest( "foreFPS" );
	if ( const auto& it = varT.find( key ); varT.cend( ) != it )
	{
		mFrameRate = it->second;
		mCountDown = 3 * mFrameRate;
	}
	const std::string scriptPathNName( "Scripts/Ready.lua" );
	const std::string varName0( "Sprite" );
	const std::string varName1( "SpriteClipWidth" );
	const std::string varName2( "SpriteClipHeight" );
	const auto table = ::ServiceLocator::LoadFromScript( scriptPathNName, varName0, varName1, varName2 );
	if ( const auto& it = table.find( varName0 ); table.cend( ) != it )
	{
		if ( false == mTexture.loadFromFile( std::get< std::string >( it->second ) ) )
		{
			// When the value has an odd path, or there's no such file,
			::global::Console( )->printError( ErrorLevel::CRITICAL, "File not found: " + varName0 + " in " + scriptPathNName );
#ifdef _DEBUG
			__debugbreak( );
#endif
		}
	}
	else
	{
		::global::Console( )->printScriptError( ErrorLevel::WARNING, varName0, scriptPathNName );
		const std::string defaultFilePathNName( "Images/Ready.png" );
		if ( false == mTexture.loadFromFile( defaultFilePathNName ) )
		{
			// When there isn't the default file,
			::global::Console( )->printError( ErrorLevel::CRITICAL, "File not found: " + defaultFilePathNName );
#ifdef _DEBUG
			__debugbreak( );
#endif
		}
	}
	bool isDebugLogOn = false;
	constexpr HashedKey key0 = ::util::hash::Digest( "debugLog" );
	if ( 0 != ::global::VariableTable( ).find( key0 )->second )
	{
		isDebugLogOn = true;
	}
	bool isDefault = true;
	// When the variable 'SpriteClipWidth' exists in the script,
	if ( const auto& it = table.find( varName1 ); table.cend( ) != it )
	{
		int temp = std::get< int >( it->second );
		// When the value is unintentionally negative,
		if ( 0 > temp )
		{
			::global::Console( )->printScriptError( ErrorLevel::WARNING, varName1, scriptPathNName );
		}
		else
		{
			mSpriteClipSize.x = temp;
			isDefault = false;
		}
	}
	if ( true == isDefault && true == isDebugLogOn )
	{
		::global::Console( )->print( "Width clipping the sprite used in main menu is set default 256." );
	}
	isDefault = true;
	// When the variable 'SpriteClipHeight' exists in the script,
	if ( const auto& it = table.find( varName2 ); table.cend( ) != it )
	{
		int temp = std::get< int >( it->second );
		// When the value is unintentionally negative,
		if ( 0 > temp )
		{
			::global::Console( )->printScriptError( ErrorLevel::WARNING, varName2, scriptPathNName );
		}
		else
		{
			mSpriteClipSize.y = temp;
			isDefault = false;
		}
	}
	if ( true == isDefault && true == isDebugLogOn )
	{
		::global::Console( )->print( "Height clipping the sprite used in main menu is set default 128." );
	}
	mSprite.setTexture( mTexture );
	mSprite.setPosition( sf::Vector2f( mWindow.getSize( ) - mSpriteClipSize ) * 0.5f );

	// Cyan
	const uint32_t BACKGROUND_RGB = 0x29cdb500u;
	const uint32_t FADE = 0x7fu;
	mBackgroundRect.setFillColor( sf::Color( BACKGROUND_RGB | FADE ) );
}

void sequence::inPlay::Ready::update( )
{
	--mCountDown;

	if ( 0 == mCountDown )
	{
		*mNextInPlaySequence = ::sequence::inPlay::Seq::PLAYING;
	}
}

void sequence::inPlay::Ready::draw( )
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
	mSprite.setTextureRect( sf::IntRect( 0, mSpriteClipSize.y*( mCountDown/mFrameRate ), mSpriteClipSize.x, mSpriteClipSize.y ) );
	mWindow.draw( mSprite );
}
