#include "../pch.h"
#include "StageView.h"
 
ui::StageView::StageView( )
	: mHasCurrentTetrimino( false ), mCellSize( 0.f )
{
	mPanel.setFillColor( sf::Color::Black );
	mTexture.loadFromFile( "Images/Ready.png" );
	mSprite.setTexture( mTexture );
}

void ui::StageView::setDimension( const sf::Vector2f position, const float cellSize )
{
	mPanel.setPosition( position );
	mCurrentTetrimino.setOrigin( position );
	const sf::Vector2f size( sf::Vector2f(::model::stage::GRID_WIDTH, ::model::stage::GRID_HEIGHT)*cellSize );
	mPanel.setSize( size );
	mSprite.setPosition( position + size*.5f );
	mCellSize = cellSize;
	mCurrentTetrimino.setSize( cellSize );
}

void ui::StageView::draw( sf::RenderWindow& window, const int time )
{
	window.draw( mPanel );
	if ( time < 0 )
	{
		mSprite.setTextureRect( sf::IntRect( 0, 256*(-time-1), 256, 256 ) );
		window.draw( mSprite );
	}
	if ( true == mHasCurrentTetrimino )
	{
		mCurrentTetrimino.draw( window );
	}
}

void ui::StageView::setCurrentTetrimino( const ::model::tetrimino::Info& info )
{
	mHasCurrentTetrimino = true;
	mCurrentTetrimino.setInfo( info );
}
