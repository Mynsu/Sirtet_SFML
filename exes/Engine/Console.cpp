#pragma hdrstop
#include "Console.h"
#include <SFML/Graphics.hpp>
#include <iostream>

ConsoleLocal::ConsoleLocal( ) : mVisible( true ),
								mCurrentInput( "_" )
								///mCursorForeground( "_" )
{
	mFont.loadFromFile( "Fonts/AGENCYB.TTF" );
	mCurrentInputTextField.setFont( mFont );
	mCurrentInputTextField.setString( mCurrentInput );
	///mCursorForegroundTextField.setFont( mFont );
	///mCursorForegroundTextField.setString( mCursorForeground );
	for ( auto& it : mHistoryTextFields )
	{
		it.setFont( mFont );
	}
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
	///mCursorForegroundTextField.setPosition( textFieldPos );
	for ( auto& it : mHistoryTextFields ) //cache
	{
		textFieldPos -= sf::Vector2f( sf::Vector2u( 0u, fontSize ) );
		it.setPosition( textFieldPos );
	}
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
				///mCursorForeground.pop_back( );
				///mCursorForeground += ' ';
				///mCursorForeground += '_';
				mCurrentInputTextField.setString( mCurrentInput );
				///mCursorForegroundTextField.setString( mCursorForeground );
			}
		}
		else if ( sf::Event::KeyPressed == event.type )
		{
			if ( sf::Keyboard::Enter == event.key.code )
			{
				auto size = mHistoryTextFields.size( );
				for ( size_t i = size - 1; i != 0; --i )
				{
					const auto& str = mHistoryTextFields[ i - 1 ].getString( );
					mHistoryTextFields[ i ].setString( str );
				}
				mCurrentInput.pop_back( );
				mHistoryTextFields[ 0 ].setString( mCurrentInput );
				mCurrentInput.clear( );
				///mCursorForeground.clear( );
				mCurrentInputTextField.setString( "" );
				///mCursorForegroundTextField.setString( '_' );
			}
			else if ( sf::Keyboard::Escape == event.key.code )
			{
				mVisible = false;
			}
			else if ( sf::Keyboard::Backspace == event.key.code )
			{
				///if ( false == mCurrentInput.empty( ) )
				if ( 1 != mCurrentInput.size( ) )
				{
					mCurrentInput.pop_back( );
					mCurrentInput.pop_back( );
					mCurrentInput += '_';
					///mCursorForeground.pop_back( );
					///mCursorForeground.pop_back( );
					///mCursorForeground += '_';
					mCurrentInputTextField.setString( mCurrentInput );
					///mCursorForegroundTextField.setString( mCursorForeground );
				}
			}
		}
	}
}

void ConsoleLocal::addCommand( std::string_view command,
							   std::function<void( void )> function,
							   CommandType type,
							   std::string_view description )
{

}
