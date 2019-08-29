#include "Console.h"
#include "ServiceLocator.h"
#include <Lib/ScriptLoader.h>

ConsoleLocal::ConsoleLocal( ) : mVisible( false ),
								mCurrentInput( "_" )
								//mCursorForeground( "_" )
{
	//
	// Mapping Exception Type to String
	//
	mExceptionTypes[ static_cast< int >( ExceptionType::VARIABLE_NOT_FOUND ) ]
		= "Variable Not Found: ";
	mExceptionTypes[ static_cast< int >( ExceptionType::TYPE_CHECK ) ]
		= "Type Check: ";
	mExceptionTypes[ static_cast< int >( ExceptionType::FILE_NOT_FOUND ) ]
		= "File Not Found: ";
	mExceptionTypes[ static_cast< int >( ExceptionType::RANGE_CHECK ) ]
		= "Range Check: ";

#ifndef _DEBUG
	try
#endif
	{
		const std::string scriptPathNName( "Scripts/Console.lua" );
		const std::string varName0( "Font" );
		const std::string varName1( "VisibleOnStart" );
		const auto result = ::util::script::LoadFromScript( scriptPathNName, varName0, varName1 );
		bool isDefault = false;
		// When there's the variable 'Font' in the script,
		if ( const auto it = result.find( varName0 ); result.cend( ) != it )
		{
			// Type check
			if ( true == std::holds_alternative< std::string >( it->second ) )
			{
				if ( false == mFont.loadFromFile( std::get< std::string >( it->second ) ) )
				{
					// File Not Found Exception
					printScriptError( ExceptionType::FILE_NOT_FOUND, varName0, scriptPathNName );
					isDefault = true;
				}
			}
			// Type Check Exception
			else
			{
				printScriptError( ExceptionType::TYPE_CHECK, varName0, scriptPathNName );
				isDefault = true;
			}
		}
		// Variable Not Found Exception
		else
		{
			printScriptError( ExceptionType::VARIABLE_NOT_FOUND, varName0, scriptPathNName );
			isDefault = true;
		}

		if ( true == isDefault )
		{
			const std::string defaultFilePathNName( "Fonts/AGENCYB.TTF" );
			if ( false == mFont.loadFromFile( defaultFilePathNName ) )
			{
				// Exception: When there's not even the default file,
				printFailure( FailureLevel::FATAL,
							  mExceptionTypes[ static_cast<int>( ExceptionType::FILE_NOT_FOUND ) ] + defaultFilePathNName );
#ifdef _DEBUG
				__debugbreak( );
#endif
			}
		}

		// When there's the variable 'VisibleOnStart' in the script,
		if ( const auto it = result.find( varName1 ); result.cend( ) != it )
		{
			// Type check
			if ( true == std::holds_alternative< bool >( it->second ) )
			{
				mVisible = std::get< bool >( it->second );
			}
			// Type Check Exception
			else
			{
				printScriptError( ExceptionType::TYPE_CHECK, varName1, scriptPathNName );
			}
		}
		/// Variable Not Found Exception
		/// else { // Nothing to do }
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
				mCurrentInput += '_';
				//mCursorForeground.clear( );
				mCurrentInputTextField.setString( mCurrentInput );
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
			//TODO: 위 방향키 누르면 최근 내역 불러오기
		}
	}
}

void ConsoleLocal::print( const std::string& message, sf::Color color )
{
	// Push up the past messages
	for ( size_t i = mHistoryTextFields.size( ) - 1; i != 0; --i )
	{
		const auto& str = mHistoryTextFields[ i - 1 ].getString( );
		const sf::Color _color = mHistoryTextFields[ i - 1 ].getFillColor( );
		mHistoryTextFields[ i ].setString( str );
		mHistoryTextFields[ i ].setFillColor( _color );
	}
	// Print the current message.
	mHistoryTextFields[ 0 ].setString( message );
	// Set font color suitable for an normal, non-error message
	if ( color != mHistoryTextFields[ 0 ].getFillColor( ) )
	{
		mHistoryTextFields[ 0 ].setFillColor( color );
	}
}

void ConsoleLocal::printFailure( const FailureLevel failureLevel, const std::string& message )
{
	// Concatenate error level info with message
	// NOTE: Using another timespace on memory is faster than in-place rearrangement in this situation.
	std::string concatenated;
	switch ( failureLevel )
	{
		case FailureLevel::WARNING:
			concatenated.assign( "WARNING: " + message );
			break;
		case FailureLevel::FATAL:
			concatenated.assign( "FATAL: " + message );
			break;
		default:
#ifdef _DEBUG
			__debugbreak( );
#else
			__assume( 0 );
			break;
#endif
	}
	print( concatenated, sf::Color::Red );
}

void ConsoleLocal::printScriptError( const ExceptionType exceptionType, const std::string& variableName, const std::string& scriptName )
{
	printFailure( FailureLevel::WARNING,
				  mExceptionTypes[ static_cast< int >( exceptionType ) ] + "[" + variableName + ":" + scriptName + "]" );
}

void ConsoleLocal::addCommand( const HashedKey command, const Command::Func& functional )
{
	mCommand.addCommand( command, functional );
}

void ConsoleLocal::processCommand( const std::string& commandLine )
{
	mCommand.processCommand( commandLine );
}

bool ConsoleLocal::isVisible( ) const
{
	return mVisible;
}
