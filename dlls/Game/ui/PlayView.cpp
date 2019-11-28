#include "../pch.h"
#include "PlayView.h"
 
ui::PlayView::PlayView( )
	: mHasCurrentTetrimino( false ), mCellSize( 0.f ),
	mWindow_( nullptr )
{
}

ui::PlayView::PlayView( sf::RenderWindow& window )
	: mHasCurrentTetrimino( false ), mCellSize( 30.f ),
	mWindow_( &window ), mStage( window )
{
	mPanel.setFillColor( sf::Color::Black );
	mTexture.loadFromFile( "Images/Ready.png" );
	mSprite.setTexture( mTexture );
}

void ui::PlayView::setDimension( const sf::Vector2f position, const float cellSize )
{
	mPanel.setPosition( position );
	mCurrentTetrimino.setOrigin( position );
	const sf::Vector2f size( sf::Vector2f(::model::stage::GRID_WIDTH, ::model::stage::GRID_HEIGHT)*cellSize );
	mPanel.setSize( size );
	mSprite.setPosition( position + size*.5f );
	mCellSize = cellSize;
	mCurrentTetrimino.setSize( cellSize );
}

void ui::PlayView::draw( const int time )
{
	mWindow_->draw( mPanel );
	if ( time < 0 )
	{
		mSprite.setTextureRect( sf::IntRect( 0, 256*(-time-1), 256, 256 ) );
		mWindow_->draw( mSprite );
	}
	if ( true == mHasCurrentTetrimino )
	{
		mCurrentTetrimino.draw( *mWindow_ );
	}
}

void ui::PlayView::updateCurrentTetrimino( const std::string& data )
{
	mCurrentTetrimino.updateOnNet( data );
	mHasCurrentTetrimino = true;
}

void ui::PlayView::updateStage( const std::string& data )
{
	mStage.updateOnNet( data );
}
