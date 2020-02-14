#include "pch.h"
#include "Console.h"
#include <Lib/ScriptLoader.h>

Console::Console( ) :
	mVisible( false ), mFontSize( 30 ), mLinesShown( 9 )
{
	mExceptionTypes[(int)ExceptionType::VARIABLE_NOT_FOUND]
		= "Variable Not Found: ";
	mExceptionTypes[(int)ExceptionType::TYPE_CHECK]
		= "Type Check: ";
	mExceptionTypes[(int)ExceptionType::FILE_NOT_FOUND]
		= "File Not Found: ";
	mExceptionTypes[(int)ExceptionType::RANGE_CHECK]
		= "Range Check: ";

	addCommand( ::util::hash::Digest("refreshcon", 10), std::bind(&Console::refresh, this, std::placeholders::_1) );
}

void Console::initialize( )
{
	uint32_t color = 0x0000ffff;
	std::string fontPathNName( "Fonts/AGENCYB.TTF" );
	mFontSize = 30;
	uint32_t currentInputFontColor = 0xffffffff;
	sf::Vector2u winSize( 800, 600 );
	mLinesShown = 9;

	const std::string scriptPathNName( "Scripts/Console.lua" );
	const std::string varName0( "VisibleOnStart" );
	const std::string varName1( "Color" );
	const std::string varName2( "Font" );
	const std::string varName3( "FontSize" );
	const std::string varName4( "CurrentInputFontColor" );
	const std::string varName5( "WinWidth" );
	const std::string varName6( "WinHeight" );
	const std::string varName7( "LinesShown" );
	const auto result = ::util::script::LoadFromScript( scriptPathNName, varName0,
													   varName1,
													   varName2, varName3,	varName4,
													   varName5, varName6,
													   varName7 );
	if ( const auto it = result.find(varName0);
		result.cend() != it )
	{
		// Type check
		if ( true == std::holds_alternative<bool>(it->second) )
		{
			mVisible = std::get<bool>(it->second);
		}
		// Type Check Exception
		else
		{
			printScriptError( ExceptionType::TYPE_CHECK, varName0, scriptPathNName );
		}
	}
	// Variable Not Found Exception
	else
	{
		printScriptError( ExceptionType::VARIABLE_NOT_FOUND, varName0, scriptPathNName );
	}

	if ( const auto it = result.find(varName1);
		result.cend() != it )
	{
		// Type check
		if ( true == std::holds_alternative<int>(it->second) )
		{
			color = std::get<int>(it->second);
		}
		// Type Check Exception
		else
		{
			printScriptError( ExceptionType::TYPE_CHECK, varName1, scriptPathNName );
		}
	}
	// Variable Not Found Exception
	else
	{
		printScriptError( ExceptionType::VARIABLE_NOT_FOUND, varName1, scriptPathNName );
	}

	if ( const auto it = result.find(varName2);
		result.cend() != it )
	{
		// Type check
		if ( true == std::holds_alternative<std::string>(it->second) )
		{
			fontPathNName = std::get<std::string>(it->second);
		}
		// Type Check Exception
		else
		{
			printScriptError( ExceptionType::TYPE_CHECK, varName2, scriptPathNName );
		}
	}
	// Variable Not Found Exception
	else
	{
		printScriptError( ExceptionType::VARIABLE_NOT_FOUND, varName2, scriptPathNName );
	}

	if ( const auto it = result.find(varName3);
		result.cend() != it )
	{
		// Type check
		if ( true == std::holds_alternative<int>(it->second) )
		{
			mFontSize = std::get<int>(it->second);
		}
		// Type Check Exception
		else
		{
			printScriptError( ExceptionType::TYPE_CHECK, varName3, scriptPathNName );
		}
	}
	// Variable Not Found Exception
	else
	{
		printScriptError( ExceptionType::VARIABLE_NOT_FOUND, varName3, scriptPathNName );
	}

	if ( const auto it = result.find(varName4);
		result.cend() != it )
	{
		// Type check
		if ( true == std::holds_alternative<int>(it->second) )
		{
			currentInputFontColor = std::get<int>(it->second);
		}
		// Type Check Exception
		else
		{
			printScriptError( ExceptionType::TYPE_CHECK, varName4, scriptPathNName );
		}
	}
	// Variable Not Found Exception
	else
	{
		printScriptError( ExceptionType::VARIABLE_NOT_FOUND, varName4, scriptPathNName );
	}

	if ( const auto it = result.find(varName5);
		result.cend() != it )
	{
		// Type check
		if ( true == std::holds_alternative<int>(it->second) )
		{
			winSize.x = std::get<int>(it->second);
		}
		// Type Check Exception
		else
		{
			printScriptError( ExceptionType::TYPE_CHECK, varName5, scriptPathNName );
		}
	}
	// Variable Not Found Exception
	else
	{
		printScriptError( ExceptionType::VARIABLE_NOT_FOUND, varName5, scriptPathNName );
	}

	if ( const auto it = result.find(varName6);
		result.cend() != it )
	{
		// Type check
		if ( true == std::holds_alternative<int>(it->second) )
		{
			winSize.y = std::get<int>(it->second);
		}
		// Type Check Exception
		else
		{
			printScriptError( ExceptionType::TYPE_CHECK, varName6, scriptPathNName );
		}
	}
	// Variable Not Found Exception
	else
	{
		printScriptError( ExceptionType::VARIABLE_NOT_FOUND, varName6, scriptPathNName );
	}

	if ( const auto it = result.find(varName7);
		result.cend() != it )
	{
		// Type check
		if ( true == std::holds_alternative<int>(it->second) )
		{
			mLinesShown = (uint8_t)std::get<int>(it->second);
		}
		// Type Check Exception
		else
		{
			printScriptError( ExceptionType::TYPE_CHECK, varName7, scriptPathNName );
		}
	}
	// Variable Not Found Exception
	else
	{
		printScriptError( ExceptionType::VARIABLE_NOT_FOUND, varName7, scriptPathNName );
	}

	mConsoleBackground.setFillColor( sf::Color(color) );
	if ( false == mFont.loadFromFile(fontPathNName) )
	{
		// File Not Found Exception
		printScriptError( ExceptionType::FILE_NOT_FOUND, varName2, scriptPathNName );
	}
	mTextFieldForCurrentInput.setFont( mFont );
	mTextFieldForCurrentInput.setCharacterSize( mFontSize );
	mTextFieldForCurrentInput.setFillColor( sf::Color(currentInputFontColor) );
	mTextFieldForCurrentInput.setString( mCurrentInput );
	mTextLabelForHistory.setFont( mFont );
	mTextLabelForHistory.setCharacterSize( mFontSize );

	const float margin = 30.f;
	const float consoleHeight =	(float)(mFontSize*(mLinesShown+1));
	mConsoleBackground.setPosition( sf::Vector2f(margin, winSize.y-consoleHeight-margin) );
	mConsoleBackground.setSize( sf::Vector2f(winSize.x-margin*2, consoleHeight+margin) );
	const sf::Vector2f curTfPos( margin+5.f, winSize.y-margin-(float)mFontSize-5.f );
	mTextFieldForCurrentInput.setPosition( curTfPos );
}

void Console::print( const std::string& message, const sf::Color color )
{
	mHistory.emplace_back( std::make_pair(message, color) );
}

void Console::printFailure( const FailureLevel failureLevel, const std::string& message )
{
	std::stringstream ss;
	switch ( failureLevel )
	{
		case FailureLevel::WARNING:
			ss << "WARNING: " << message;
			break;
		case FailureLevel::FATAL:
			ss << "FATAL: " << message;
			break;
		default:
#ifndef _DEBUG
			__assume(0);
#endif
			break;
	}
	print( ss.str(), sf::Color::Red );
}

void Console::printScriptError( const ExceptionType exceptionType,
							   const std::string& variableName,
							   const std::string& scriptName )
{
	printFailure( FailureLevel::WARNING,
				  mExceptionTypes[(int)exceptionType]+'['+variableName+':'+scriptName+']' );
}

void Console::addCommand( const HashedKey command, const Func& functional )
{
	mCommand.addCommand( command, functional );
}

void Console::removeCommand( const HashedKey command )
{
	mCommand.removeCommand( command );
}

void Console::draw( sf::RenderWindow& window )
{
	window.draw( mConsoleBackground );
	window.draw( mTextFieldForCurrentInput );
	sf::Vector2f textFieldPos( mTextFieldForCurrentInput.getPosition() );
	uint8_t count = 0;
	for ( auto crit = mHistory.crbegin(); mHistory.crend() != crit; ++crit )
	{
		textFieldPos -= sf::Vector2f(sf::Vector2u(0, mFontSize));
		mTextLabelForHistory.setPosition( textFieldPos );
		mTextLabelForHistory.setString( crit->first );
		mTextLabelForHistory.setFillColor( crit->second );
		window.draw( mTextLabelForHistory );
		if ( mLinesShown <= ++count )
		{
			break;
		}
	}
}

void Console::handleEvent( std::vector<sf::Event>& eventQueue )
{
	for ( auto it = eventQueue.cbegin(); eventQueue.cend() != it; )
	{
		if ( sf::Event::KeyPressed == it->type && sf::Keyboard::Tab == it->key.code )
		{
			// Toggling console on/off
			mVisible = !mVisible;
			eventQueue.erase(it);
			return;
		}

		if ( true == mVisible )
		{
			if ( sf::Event::TextEntered == it->type )
			{
				if ( uint32_t input = (uint32_t)it->text.unicode;
					 input > 0x1f && input < 0x7f )
				{
					mCurrentInput += (char)input;
					mTextFieldForCurrentInput.setString( mCurrentInput + '_' );
				}
			}
			else if ( sf::Event::KeyPressed == it->type )
			{
				switch ( it->key.code )
				{
					case sf::Keyboard::Escape:
						mVisible = false;
						break;
					case sf::Keyboard::Enter:
						print( mCurrentInput );
						mCommand.processCommand( mCurrentInput );
						mCurrentInput.clear( );
						mTextFieldForCurrentInput.setString( mCurrentInput + '_' );
						break;
					case sf::Keyboard::Backspace:
						if ( false == mCurrentInput.empty() )
						{
							mCurrentInput.pop_back( );
							mTextFieldForCurrentInput.setString( mCurrentInput + '_' );
						}
						break;
					case sf::Keyboard::Up:
						mCurrentInput = mHistory.back().first;
						mTextFieldForCurrentInput.setString( mCurrentInput + '_' );
						break;
					case sf::Keyboard::Down:
						mCurrentInput.clear( );
						mTextFieldForCurrentInput.setString( mCurrentInput + '_' );
						break;
					default:
						break;
				}
			}
			it = eventQueue.erase(it);
		}
		else
		{
			++it;
		}
	}
}

bool Console::isVisible( ) const
{
	return mVisible;
}