#include "../pch.h"
#include "PlayView.h"
#include "../scene/online/Online.h"
#include "../ServiceLocatorMirror.h"
 
ui::PlayView::PlayView( )
	: mHasStarted( false ), mCellSize( 0.f ),
	mWindow_( nullptr ), mNet( nullptr )
{
}

ui::PlayView::PlayView( sf::RenderWindow& window, ::scene::online::Online& net )
	: mHasStarted( false ), mCellSize( 30.f ),
	mWindow_( &window ), mNet( &net ), mStage( window )
{
	if ( false == mTexture.loadFromFile("Images/Ready.png") )
	{
		gService()->console().printFailure( FailureLevel::WARNING, "Can't find the countdown image." );
	}
	mSprite.setTexture( mTexture );
	mSprite.setOrigin( 128.f, 128.f );
}

void ui::PlayView::setDimension( const sf::Vector2f position, const float cellSize )
{
	mCellSize = cellSize;
	const sf::Vector2f size( sf::Vector2f(::model::stage::GRID_WIDTH, ::model::stage::GRID_HEIGHT)*cellSize );
	mSprite.setPosition( position + size*.5f );
	mCurrentTetrimino.setOrigin( position );
	mCurrentTetrimino.setSize( cellSize );
	mStage.setPosition( position );
	mStage.setSize( cellSize );
}

void ui::PlayView::update( )
{
	if ( std::optional<std::string> stage(mNet->getByTag(TAG_MY_STAGE,
														 ::scene::online::Online::Option::SERIALIZED));
		std::nullopt != stage )
	{
		mStage.updateOnNet( stage.value() );
	}

	if ( std::optional<std::string> curTet(mNet->getByTag(TAG_MY_CURRENT_TETRIMINO,
														  ::scene::online::Online::Option::SERIALIZED));
		std::nullopt != curTet )
	{
		mCurrentTetrimino.updateOnNet( curTet.value() );
		mHasStarted = true;
	}

	mNet->sendZeroByte( );
}

void ui::PlayView::draw( const int time )
{
	mStage.draw( );
	if ( time < 0 )
	{
		//mSprite.setTextureRect( sf::IntRect( 0, 256*(-time-1), 256, 256 ) );
		mSprite.setTextureRect( sf::IntRect( 0, 0, 256, 256 ) );
		mWindow_->draw( mSprite );
	}
	if ( true == mHasStarted )
	{
		mCurrentTetrimino.draw( *mWindow_ );
	}
}