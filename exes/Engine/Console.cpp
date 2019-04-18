#pragma hdrstop
#include "Console.h"
#include <SFML\Graphics.hpp>
//TODO:
#include <iostream>

ConsoleLocal::ConsoleLocal( ) : mVisible( true ),
								mCurrentInput( "" ),
								mCursorPosition( 0 ),
								mCursorForeground( "_" )
{
	mFont.loadFromFile( "Fonts\AGENCYR.TTF" );
	mCurrentInputTextField.setFont( mFont );
	//mCurrentInputTextField.setPosition( 0.0f, 500.0f );//
	//mCurrentInputTextField.setFillColor( sf::Color::White );
	
	//mCursorForegroundTextField.setFillColor( sf::Color::White );
}

void ConsoleLocal::draw( sf::RenderTarget& target, sf::RenderStates states ) const
{
	auto targetSize = target.getSize( );
	///sf::View view( sf::FloatRect( sf::Vector2f( 0.0f, 0.0f ),
	///							  sf::Vector2f( sf::Vector2i( targetSize.x, targetSize.y ) ) ) );
	///target.setView( view );

	sf::RectangleShape rect;
	rect.setPosition( sf::Vector2f( sf::Vector2i( 0, targetSize.y / 2 ) ) );
	rect.setSize( sf::Vector2f( sf::Vector2i( targetSize.x / 2, targetSize.y / 2 ) ) );
	rect.setOutlineColor( sf::Color::White );
	rect.setFillColor( sf::Color::Blue );
	target.draw( rect );

	target.draw( mCurrentInputTextField );
	target.draw( mCursorForegroundTextField );

	///target.setView( target.getDefaultView( ) );
}

void ConsoleLocal::handleEvent( const sf::Event& event )
{
	if ( sf::Event::TextEntered == event.type )
	{
		auto input = event.text.unicode;
		//if ( input > 0x1f && input < 0x7d )
		{
			mCurrentInput += static_cast< char >( event.text.unicode );
			mCursorForeground.pop_back( );
			mCursorForeground += " ";
			mCursorForeground += "_";

			mCurrentInputTextField.setString( mCurrentInput );
			mCursorForegroundTextField.setString( mCursorForeground );
		}
	}
}

//void ConsoleLocal::update( )
//{
//	mCurrentInputTextField.setString( mCurrentInput );
//
//	mCursorForegroundTextField.setString( mCursorForeground );
//}

bool ConsoleLocal::isVisible( ) const
{
	return mVisible;
}

void ConsoleLocal::toggleVisible( )
{
	mVisible = !mVisible;
}

void ConsoleLocal::addCommand( std::string_view command,
							   std::function<void( void )> function,
							   CommandType type,
							   std::string_view description )
{

}
