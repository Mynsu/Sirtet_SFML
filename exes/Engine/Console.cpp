#include "Console.h"
#include <iostream>
#include <SFML/Graphics.hpp>
#include "ServiceLocator.h"

ConsoleLocal::ConsoleLocal( ) : mVisible( false ),
								mInitialized( false ),
								mCurrentInput( "_" )
								//mCursorForeground( "_" )
{
	try
	{
		const std::string scriptPathNName( "Scripts/Console.lua" );
		const std::string varName0( "Font" );
		const std::string varName1( "Visible" );
		const auto values = ServiceLocator::LoadFromScript( scriptPathNName, varName0, varName1 );
		mFont.loadFromFile( std::get< std::string >( values.at( varName0 ) ) );//궁금:std::string으로 복사해서 get하는 거 아니겠지?
		if ( const auto& it = values.find( varName1 ); values.cend( ) != it )
		{
			mVisible = std::get< bool >( it->second );
		}
	}
	catch ( std::runtime_error& )
	{ }
	mCurrentInputTextField.setFont( mFont );
	mCurrentInputTextField.setString( mCurrentInput );
	//mCursorForegroundTextField.setFont( mFont );
	//mCursorForegroundTextField.setString( mCursorForeground );
	for ( auto& it : mHistoryTextFields )
	{
		it.setFont( mFont );
	}

	///addCommand( "refresh", &ConsoleLocal::refresh, CommandType::SYSTEM, "Reload console's new font, vars, etc." );
}

void ConsoleLocal::init( const sf::Vector2u& winSize )
{
	const sf::Vector2f _winSize( winSize );
	const float margin = 30.f;
	const float consoleRatio = 2.f;
	mConsoleWindow.setPosition( sf::Vector2f( margin, _winSize.y / consoleRatio - margin ) );
	mConsoleWindow.setSize( _winSize / consoleRatio );
	mConsoleWindow.setOutlineColor( sf::Color::White );
	mConsoleWindow.setFillColor( sf::Color::Blue );
	const auto fontSize = mCurrentInputTextField.getCharacterSize( );
	sf::Vector2f textFieldPos( margin, winSize.y - margin - fontSize );
	mCurrentInputTextField.setPosition( textFieldPos );
	//mCursorForegroundTextField.setPosition( textFieldPos );
	for ( auto& it : mHistoryTextFields ) //cache
	{
		textFieldPos -= sf::Vector2f( sf::Vector2u( 0u, fontSize ) );
		it.setPosition( textFieldPos );
	}

	mInitialized = true;
}

void ConsoleLocal::draw( sf::RenderTarget& target, sf::RenderStates states ) const
{
	//sf::View view( sf::FloatRect( sf::Vector2f( 0.0f, 0.0f ),
	//							  sf::Vector2f( target.getSize( ) ) ) );
	//target.setView( view );

	target.draw( mConsoleWindow );
	target.draw( mCurrentInputTextField );
	///target.draw( mCursorForegroundTextField );
	for ( const auto it : mHistoryTextFields ) //cache
	{
		target.draw( it );
	}

	//target.setView( target.getDefaultView( ) );
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
				/*auto size = mHistoryTextFields.size( );
				for ( size_t i = size - 1; i != 0; --i )
				{
					const auto& str = mHistoryTextFields[ i - 1 ].getString( );
					mHistoryTextFields[ i ].setString( str );
				}*/
				mCurrentInput.pop_back( );
				//mHistoryTextFields[ 0 ].setString( mCurrentInput );
				print( mCurrentInput );
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

void ConsoleLocal::print( const std::string& message )
{
	auto size = mHistoryTextFields.size( );
	for ( size_t i = size - 1; i != 0; --i )
	{
		const auto& str = mHistoryTextFields[ i - 1 ].getString( );
		mHistoryTextFields[ i ].setString( str );
	}
	mHistoryTextFields[ 0 ].setString( message );
	if ( sf::Color::White != mHistoryTextFields[ 0 ].getFillColor( ) )
	{
		mHistoryTextFields[ 0 ].setFillColor( sf::Color::White );
	}
}

void ConsoleLocal::printError( const std::string& errorMessage )
{
	auto size = mHistoryTextFields.size( );
	for ( size_t i = size - 1; i != 0; --i )
	{
		const auto& str = mHistoryTextFields[ i - 1 ].getString( );
		mHistoryTextFields[ i ].setString( str );
	}
	mHistoryTextFields[ 0 ].setString( errorMessage );
	// NOTE: An error seldom occurs,
	// thus checking out has been passed away for the performance.
	///if ( sf::Color::Red != mHistoryTextFields[ 0 ].getFillColor( ) )
	///{
		mHistoryTextFields[ 0 ].setFillColor( sf::Color::Red );
	///}

	// NOTE: Encouraging performance is prior to saving memory in modern game dev.
	// In this case, Duplicate codes seems tolerable rather than context-switch cost.
	///print( "ERROR: " + errorMessage );
}