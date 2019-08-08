#include "MainMenu.h"
#include <Lib/ScriptLoader.h>

namespace sequence
{
	bool MainMenu::IsInstanciated = false;

	MainMenu::MainMenu( sf::RenderWindow& window )
		: mOnIndicator( ::sequence::Seq::NONE ),
		mWindow(window),
		mSpriteClipSize( 256, 128 )
	{
		ASSERT_FALSE( IsInstanciated );
		const std::string scriptPathNName( "Scripts/MainMenu.lua" );
		const std::string varName0( "Sprite" );
		const std::string varName1( "SpriteClipWidth" );
		const std::string varName2( "SpriteClipHeight" );
		const auto table = ::util::script::LoadFromScript( scriptPathNName, varName0, varName1, varName2 );
		// When the variable 'Sprite' exists in the script,
		if ( const auto& it = table.find( varName0 ); table.cend( ) != it )
		{
			if ( false == mTexture.loadFromFile( std::get< std::string >( it->second ) ) )
			{
				// Exception: When the value has an odd path, or there's no such file,
				::global::Console( )->printFailure( FailureLevel::CRITICAL, "File not found: " + varName0 + " in " + scriptPathNName );
#ifdef _DEBUG
				__debugbreak( );
#endif
			}
		}
		// Exception: When the variable 'Sprite' doesN'T exist in the script,
		else
		{
			::global::Console( )->printScriptError( FailureLevel::WARNING, varName0, scriptPathNName );
			const std::string defaultFilePathNName( "Images/MainMenu.png" );
			if ( false == mTexture.loadFromFile( defaultFilePathNName ) )
			{
				// Exception: When there isn't the default file,
				::global::Console( )->printFailure( FailureLevel::CRITICAL, "File not found: " + defaultFilePathNName );
#ifdef _DEBUG
				__debugbreak( );
#endif
			}
		}
		bool isDefault = true;
		// When the variable 'SpriteClipWidth' exists in the script,
		if ( const auto& it = table.find( varName1 ); table.cend( ) != it )
		{
			const int temp = std::get< int >( it->second );
			// Exception: When the value is unintentionally negative,
			if ( 0 > temp )
			{
				::global::Console( )->printScriptError( FailureLevel::WARNING, varName1, scriptPathNName );
			}
			// When the value looks OK,
			else
			{
				mSpriteClipSize.x = temp;
				isDefault = false;
			}
		}
		if ( true == isDefault )
		{
			::global::Console( )->print( "Width clipping the MAIN MENU sprite is set default 256." );
		}
		isDefault = true;
		// When the variable 'SpriteClipHeight' exists in the script,
		if ( const auto& it = table.find( varName2 ); table.cend( ) != it )
		{
			int temp = std::get< int >( it->second );
			// Exception: When the value is unintentionally negative,
			if ( 0 > temp )
			{
				::global::Console( )->printScriptError( FailureLevel::WARNING, varName2, scriptPathNName );
			}
			// When the value looks OK,
			else
			{
				mSpriteClipSize.y = temp;
				isDefault = false;
			}
		}
		if ( true == isDefault )
		{
			::global::Console( )->print( "Height clipping the MAIN MENU sprite is set default 128." );
		}
		mSprite.setTexture( mTexture );
		IsInstanciated = true;
	}

	void MainMenu::update( )
	{
		if ( true == sf::Mouse::isButtonPressed( sf::Mouse::Left ) )
		{
			switch ( mOnIndicator )
			{
				case ::sequence::Seq::SINGLE_PLAY:
					*mNextMainSequence = mOnIndicator;
					break;
				default:
					break;
			}
		}
	}

	void MainMenu::draw( )
	{
		const sf::Vector2f winSize( mWindow.getSize( ) );
		const sf::Vector2f logoMargin( 80.f, 80.f );
		// Down right
		mSprite.setPosition( winSize - sf::Vector2f( mSpriteClipSize ) - logoMargin );
		mSprite.setTextureRect( sf::IntRect( 0, 0, mSpriteClipSize.x, mSpriteClipSize.y ) );
		mWindow.draw( mSprite );

		// Vertical middle
		mSprite.setPosition( winSize / 3.2f );
		if ( true == mSprite.getGlobalBounds( ).contains( sf::Vector2f( sf::Mouse::getPosition( mWindow ) ) ) )
		{
			mOnIndicator = ::sequence::Seq::SINGLE_PLAY;
			mSprite.setTextureRect( sf::IntRect( 0, 2*mSpriteClipSize.y, mSpriteClipSize.x, mSpriteClipSize.y ) );
		}
		else
		{
			mOnIndicator = ::sequence::Seq::NONE;
			mSprite.setTextureRect( sf::IntRect( 0, mSpriteClipSize.y, mSpriteClipSize.x, mSpriteClipSize.y ) );
		}
		mWindow.draw( mSprite );
	}
}
