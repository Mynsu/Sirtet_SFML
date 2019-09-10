#include "MainMenu.h"
#include <Lib/ScriptLoader.h>
#include "../ServiceLocatorMirror.h"

bool ::scene::MainMenu::IsInstantiated = false;

::scene::MainMenu::MainMenu( sf::RenderWindow& window, const SetScene_t& setScene )
	: mWindow( window ), mSetScene( setScene ),
	mOnIndicator( ::scene::ID::MAX_NONE ),
	mSpriteClipSize_( 256u, 128u )
{
	ASSERT_FALSE( IsInstantiated );

	loadResources( );

	IsInstantiated = true;
}

::scene::MainMenu::~MainMenu( )
{
	IsInstantiated = false;
}

void scene::MainMenu::loadResources( )
{
	const std::string scriptPathNName( "Scripts/MainMenu.lua" );
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
		const std::string defaultFilePathNName( "Images/MainMenu.png" );
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
			// Range check
			if ( const int second = std::get< int >( it->second ); 0 <= second )
			{
				mSpriteClipSize_.x = second;
				isDefault = false;
			}
			// Range Check Exception
			else
			{
				ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::RANGE_CHECK, varName1, scriptPathNName );
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
		ServiceLocatorMirror::Console( )->print( "Width clipping the MAIN_MENU sprite is set 256." );
	}

	isDefault = true;
	// When there's the variable 'SpriteClipHeight' in the script,
	if ( const auto it = result.find( varName2 ); result.cend( ) != it )
	{
		// Type check
		if ( true == std::holds_alternative< int >( it->second ) )
		{
			// Range check
			if ( const int second = std::get< int >( it->second ); 0 <= second )
			{
				mSpriteClipSize_.y = second;
				isDefault = false;
			}
			// Range Check Exception
			else
			{
				ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::RANGE_CHECK, varName2, scriptPathNName );
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
		ServiceLocatorMirror::Console( )->print( "Height clipping the MAIN_MENU sprite is set 128." );
	}
	mSprite.setTexture( mTexture );
}

void ::scene::MainMenu::update( std::queue< sf::Event >& )
{
	if ( true == sf::Mouse::isButtonPressed( sf::Mouse::Left ) )
	{
		if ( ::scene::ID::SINGLE_PLAY == mOnIndicator )
		{
			mSetScene( mOnIndicator );
		}
	}
}

void ::scene::MainMenu::draw( )
{
	const sf::Vector2f winSize( mWindow.getSize( ) );
	const sf::Vector2f logoMargin( 70.f, 70.f );
	// Bottom right on screen
	const sf::Vector2i logoSize( mSpriteClipSize_.x, 2*mSpriteClipSize_.y );
	mSprite.setPosition( winSize - sf::Vector2f(logoSize) - logoMargin );
	mSprite.setTextureRect( sf::IntRect(sf::Vector2i(0,0), logoSize) );
	mWindow.draw( mSprite );

	// Vertical middle on screen
	mSprite.setPosition( winSize / 4.f );
	mSprite.setTextureRect( sf::IntRect(sf::Vector2i(0,0),sf::Vector2i(mSpriteClipSize_)) );
	if ( true == mSprite.getGlobalBounds( ).contains( sf::Vector2f( sf::Mouse::getPosition( mWindow ) ) ) )
	{
		mOnIndicator = ::scene::ID::SINGLE_PLAY;
		mSprite.setTextureRect( sf::IntRect( mSpriteClipSize_.x, 2*mSpriteClipSize_.y, mSpriteClipSize_.x, mSpriteClipSize_.y ) );
	}
	else
	{
		mOnIndicator = ::scene::ID::MAX_NONE;
		mSprite.setTextureRect( sf::IntRect( 0, 2*mSpriteClipSize_.y, mSpriteClipSize_.x, mSpriteClipSize_.y ) );
	}
	mWindow.draw( mSprite );
}

::scene::ID scene::MainMenu::currentScene( ) const
{
	return ::scene::ID::MAIN_MENU;
}