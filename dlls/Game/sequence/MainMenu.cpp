#include "MainMenu.h"
#include <Lib/ScriptLoader.h>

namespace sequence
{
	namespace
	{
		// At a time lives a single instance of this type, two or more can't.
		// NOTE: Global access isn't necessary here.
		bool IsInstanciated = false;
	}

	MainMenu::MainMenu( sf::RenderWindow& window, const SetSequence_t& setSequence )
		: mOnIndicator( ::sequence::Seq::MAX_NONE ),
		mWindow( window ),
		mSetSequence( setSequence ),
		mSpriteClipSize( 256u, 128u )
	{
		ASSERT_FALSE( IsInstanciated );

		const std::string scriptPathNName( "Scripts/MainMenu.lua" );
		const std::string varName0( "Sprite" );
		const std::string varName1( "SpriteClipWidth" );
		const std::string varName2( "SpriteClipHeight" );
		const auto result = ::util::script::LoadFromScript( scriptPathNName, varName0, varName1, varName2 );
		// When the variable 'Sprite' exists in the script,
		if ( const auto it = result.find( varName0 ); result.cend( ) != it )
		{
			// Type check
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
			// Type Check Exception
			else
			{
				ServiceLocatorMirror::Console( )->printScriptError( FailureLevel::WARNING, varName0, scriptPathNName );
			}
		}
		// Exception: When the variable 'Sprite' doesN'T exist in the script,
		else
		{
			ServiceLocatorMirror::Console( )->printScriptError( FailureLevel::WARNING, varName0, scriptPathNName );
			const std::string defaultFilePathNName( "Images/MainMenu.png" );
			if ( false == mTexture.loadFromFile( defaultFilePathNName ) )
			{
				// Exception: When there isn't the default file,
				ServiceLocatorMirror::Console( )->printFailure( FailureLevel::CRITICAL, "File not found: " + defaultFilePathNName );
#ifdef _DEBUG
				__debugbreak( );
#endif
			}
		}
		bool isDefault = true;
		// When the variable 'SpriteClipWidth' exists in the script,
		if ( const auto it = result.find( varName1 ); result.cend( ) != it )
		{
			// Type check
			if ( true == std::holds_alternative< int >( it->second ) )
			{
				// Range check
				if ( const int second = std::get< int >( it->second ); 0 <= second )
				{
					mSpriteClipSize.x = second;
					isDefault = false;
				}
				// Range Check Exception
				else
				{
					ServiceLocatorMirror::Console( )->printScriptError( FailureLevel::WARNING, varName1, scriptPathNName );
				}
			}
			// Type Check Exception
			else
			{
				ServiceLocatorMirror::Console( )->printScriptError( FailureLevel::WARNING, varName1, scriptPathNName );
			}
		}
		if ( true == isDefault )
		{
			ServiceLocatorMirror::Console( )->print( "Width clipping the MAIN MENU sprite is set default 256." );
		}
		isDefault = true;
		// When the variable 'SpriteClipHeight' exists in the script,
		if ( const auto it = result.find( varName2 ); result.cend( ) != it )
		{
			// Type check
			if ( true == std::holds_alternative< int >( it->second ) )
			{
				// Range check
				if ( const int second = std::get< int >( it->second ); 0 <= second )
				{
					mSpriteClipSize.y = second;
					isDefault = false;
				}
				// Range Check Exception
				else
				{
					ServiceLocatorMirror::Console( )->printScriptError( FailureLevel::WARNING, varName2, scriptPathNName );
				}
			}
			else
			{
					ServiceLocatorMirror::Console( )->printScriptError( FailureLevel::WARNING, varName2, scriptPathNName );
			}
		}
		if ( true == isDefault )
		{
			ServiceLocatorMirror::Console( )->print( "Height clipping the MAIN MENU sprite is set default 128." );
		}
		mSprite.setTexture( mTexture );

		IsInstanciated = true;
	}

	MainMenu::~MainMenu( )
	{
		IsInstanciated = false;
	}

	void MainMenu::update( )
	{
		if ( true == sf::Mouse::isButtonPressed( sf::Mouse::Left ) )//TODO: 창 밖에 클릭해도 해당되는 버그
		{
			mSetSequence( mOnIndicator );
		}
	}

	void MainMenu::draw( )
	{
		const sf::Vector2f winSize( mWindow.getSize( ) );
		const sf::Vector2f logoMargin( 80.f, 80.f );
		// Bottom right on screen
		mSprite.setPosition( winSize - sf::Vector2f( mSpriteClipSize ) - logoMargin );
		mSprite.setTextureRect( sf::IntRect( 0, 0, mSpriteClipSize.x, mSpriteClipSize.y ) );
		mWindow.draw( mSprite );

		// Vertical middle on screen
		mSprite.setPosition( winSize / 3.2f );
		if ( true == mSprite.getGlobalBounds( ).contains( sf::Vector2f( sf::Mouse::getPosition( mWindow ) ) ) )
		{
			mOnIndicator = ::sequence::Seq::SINGLE_PLAY;
			mSprite.setTextureRect( sf::IntRect( 0, 2*mSpriteClipSize.y, mSpriteClipSize.x, mSpriteClipSize.y ) );
		}
		else
		{
			mOnIndicator = ::sequence::Seq::MAX_NONE;
			mSprite.setTextureRect( sf::IntRect( 0, mSpriteClipSize.y, mSpriteClipSize.x, mSpriteClipSize.y ) );
		}
		mWindow.draw( mSprite );
	}
	auto MainMenu::newEqualTypeInstance( ) -> std::unique_ptr<::sequence::ISequence>
	{
		IsInstanciated = false;
		return std::make_unique< ::sequence::MainMenu >( mWindow, mSetSequence );
	}
}
