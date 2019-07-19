#include "MainMenu.h"

namespace sequence
{
	bool MainMenu::IsInstanciated = false;

	MainMenu::MainMenu( sf::RenderWindow& window,
						::sequence::Seq* const nextMainSequence )
		: mOnIndicator( ::sequence::Seq::NONE ),
		mNextMainSequence( nextMainSequence ),
		mWindow(window),
		mSpriteClipSize( 256, 128 )
	{
		ASSERT_FALSE( IsInstanciated );
		*mNextMainSequence = ::sequence::Seq::NONE;
		const std::string scriptPathNName( "Scripts/MainMenu.lua" );
		const std::string varName0( "Sprite" );
		const std::string varName1( "SpriteClipWidth" );
		const std::string varName2( "SpriteClipHeight" );
		const auto table = ::ServiceLocator::LoadFromScript( scriptPathNName, varName0, varName1, varName2 );
		// When the variable 'Sprite' exists in the script,
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
		// When the variable 'Sprite' doesN'T exist in the script,
		else
		{
			::global::Console( )->printScriptError( ErrorLevel::WARNING, varName0, scriptPathNName );
			const std::string defaultFilePathNName( "Images/MainMenu.png" );
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
