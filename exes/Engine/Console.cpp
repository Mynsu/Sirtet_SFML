#include "Console.h"
#include "ServiceLocator.h"
#include <Lib/ScriptLoader.h>

ConsoleLocal::ConsoleLocal( ) : mVisible( false ),
								mCurrentInput( "_" )
								//mCursorForeground( "_" )
{
#ifndef _DEBUG
	try
#endif
	{
		const std::string scriptPathNName( "Scripts/Console.lua" );
		const std::string varName0( "Font" );
		const std::string varName1( "VisibleOnStart" );
		const auto table = ::util::script::LoadFromScript( scriptPathNName, varName0, varName1 );
		// When the variable 'Font' exists in the script,
		if ( const auto& it = table.find( varName0 ); table.cend( ) != it )
		{
			if ( false == mFont.loadFromFile( std::get< std::string >( it->second ) ) )
			{
				// Exception: When the value has an odd path, or there's no such file,
				printFailure( FailureLevel::CRITICAL, "File not found: " + varName0 + " in " + scriptPathNName );
#ifdef _DEBUG
				__debugbreak( );
#endif
			}
		}
		// Exception: When the variable 'Font' doesN'T exist in the script,
		else
		{
			printScriptError( FailureLevel::WARNING, varName0, scriptPathNName );
			const std::string defaultFilePathNName( "Fonts/AGENCYB.TTF" );
			if ( false == mFont.loadFromFile( defaultFilePathNName ) )
			{
				// Exception: When there isn't the default file,
				printFailure( FailureLevel::CRITICAL, "File not found: " + defaultFilePathNName );
#ifdef _DEBUG
				__debugbreak( );
#endif
			}
		}
		// When the variable 'VisibleOnStart' exists in the script,
		if ( const auto& it = table.find( varName1 ); table.cend( ) != it )
		{
			mVisible = std::get< bool >( it->second );
		}
	}
#ifndef _DEBUG
	catch ( std::runtime_error& )
	{
		// Nothing to do
	}
#endif
	mCurrentInputTextField.setFont( mFont );
	mCurrentInputTextField.setString( mCurrentInput );
	//mCursorForegroundTextField.setFont( mFont );
	//mCursorForegroundTextField.setString( mCursorForeground );
	for ( auto& it : mHistoryTextFields )
	{
		it.setFont( mFont );
	}

	mConsoleWindow.setOutlineColor( sf::Color::White );
	mConsoleWindow.setOutlineThickness( 0.2f );
	mConsoleWindow.setFillColor( sf::Color::Blue );
	setPosition( sf::Vector2u( 800u, 600u ) );
}

void ConsoleLocal::setPosition( const sf::Vector2u& winSize )
{
	const sf::Vector2f _winSize( winSize );
	const float margin = 30.f;
	const float consoleRatio = 0.5f;
	mConsoleWindow.setPosition( sf::Vector2f( margin, _winSize.y*consoleRatio-margin ) );
	mConsoleWindow.setSize( sf::Vector2f( _winSize.x-margin*2, _winSize.y*consoleRatio ) );
	const auto fontSize = mCurrentInputTextField.getCharacterSize( );
	sf::Vector2f textFieldPos( margin+5.f, _winSize.y - margin - static_cast<float>( fontSize ) );
	mCurrentInputTextField.setPosition( textFieldPos );
	//mCursorForegroundTextField.setPosition( textFieldPos );
	for ( auto& it : mHistoryTextFields )
	{
		textFieldPos -= sf::Vector2f( sf::Vector2u( 0u, fontSize ) );
		it.setPosition( textFieldPos );
	}
}

void ConsoleLocal::draw( sf::RenderTarget& target, sf::RenderStates states ) const
{
	target.draw( mConsoleWindow );
	target.draw( mCurrentInputTextField );
	///target.draw( mCursorForegroundTextField );
	for ( const auto& it : mHistoryTextFields )
	{
		target.draw( it );
	}
}

void ConsoleLocal::handleEvent( const sf::Event& event )
{
	if ( sf::Event::KeyPressed == event.type )
	{
		if ( sf::Keyboard::Tab == event.key.code )
		{
			// Toggle console on/off
			mVisible = !mVisible;
			return;
		}
	}

	if ( true == mVisible )
	{
		if ( sf::Event::TextEntered == event.type )
		{
			if ( auto input = event.text.unicode;
				 input > 0x1f && input < 0x7f )
			{
				mCurrentInput.pop_back( );
				mCurrentInput += static_cast< char >( input );
				mCurrentInput += '_';
				//mCursorForeground.pop_back( );
				//mCursorForeground += ' ';
				//mCursorForeground += '_';
				mCurrentInputTextField.setString( mCurrentInput );
				//mCursorForegroundTextField.setString( mCursorForeground );
			}
		}
		else if ( sf::Event::KeyPressed == event.type )
		{
			if ( sf::Keyboard::Enter == event.key.code )
			{
				mCurrentInput.pop_back( );
				print( mCurrentInput );
				mCommand.processCommand( mCurrentInput );
				mCurrentInput.clear( );
				//mCursorForeground.clear( );
				mCurrentInputTextField.setString( "" );
				//mCursorForegroundTextField.setString( '_' );
			}
			else if ( sf::Keyboard::Escape == event.key.code )
			{
				mVisible = false;
			}
			else if ( sf::Keyboard::Backspace == event.key.code )
			{
				//if ( false == mCurrentInput.empty( ) )
				if ( 1 != mCurrentInput.size( ) )
				{
					mCurrentInput.pop_back( );
					mCurrentInput.pop_back( );
					mCurrentInput += '_';
					//mCursorForeground.pop_back( );
					//mCursorForeground.pop_back( );
					//mCursorForeground += '_';
					mCurrentInputTextField.setString( mCurrentInput );
					//mCursorForegroundTextField.setString( mCursorForeground );
				}
			}
		}
	}
}

void ConsoleLocal::print( const std::string& message, sf::Color color )
{
	// Push up the past messages
	for ( size_t i = mHistoryTextFields.size( ) - 1; i != 0; --i )
	{
		const auto& str = mHistoryTextFields[ i - 1 ].getString( );
		mHistoryTextFields[ i ].setString( str );
	}
	// Print the current message.
	mHistoryTextFields[ 0 ].setString( message );
	// Set font color suitable for an normal, non-error message
	if ( color != mHistoryTextFields[ 0 ].getFillColor( ) )
	{
		mHistoryTextFields[ 0 ].setFillColor( color );
	}
}