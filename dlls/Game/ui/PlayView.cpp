#include "../pch.h"
#include "PlayView.h"
#include "../scene/online/Online.h"
#include "../ServiceLocatorMirror.h"
 
ui::PlayView::PlayView( )
	: mHasCurrentTetrimino( false ), mCellSize( 0.f ),
	mWindow_( nullptr ), mNet( nullptr )
{
}

ui::PlayView::PlayView( sf::RenderWindow& window, ::scene::online::Online& net )
	: mHasCurrentTetrimino( false ), mCellSize( 30.f ),
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

void ui::PlayView::update( std::list<sf::Event>& eventQueue )
{
	if ( std::optional<std::string> stage(mNet->getByTag(TAG_MY_STAGE,
														 ::scene::online::Online::Option::SERIALIZED));
		std::nullopt != stage )
	{
		mStage.updateOnNet( stage.value() );
		mHasCurrentTetrimino = false;
	}
	else if ( std::optional<std::string> curTet(mNet->getByTag(TAG_MY_CURRENT_TETRIMINO,
														  ::scene::online::Online::Option::SERIALIZED));
		std::nullopt != curTet )
	{
		mCurrentTetrimino.updateOnNet( curTet.value() );
		mHasCurrentTetrimino = true;
	}
	
	std::string input;
	for ( auto it = eventQueue.cbegin(); eventQueue.cend() != it; )
	{
		if ( sf::Event::KeyPressed == it->type )
		{
			switch ( it->key.code )
			{
				case sf::Keyboard::Space:
					input += TAG_TETRIMINO_MOVE + std::to_string( (int)::model::tetrimino::Move::FALL_DOWN );
					//mCurrentTetrimino.fallDown( );
					// NOTE: Don't 'return', or it can't come out of the infinite loop.
					///return;
					//[[ fallthrough ]];
					break;
				case sf::Keyboard::Down:
					input += TAG_TETRIMINO_MOVE + std::to_string( (int)::model::tetrimino::Move::DOWN );
					//hasCollidedAtThisFrame = mCurrentTetrimino.moveDown( mPlayerStage.cgrid( ) );
					//mFrameCount_fallDown = 0u;
					it = eventQueue.erase( it );
					break;
				case sf::Keyboard::Left:
					input += TAG_TETRIMINO_MOVE + std::to_string( (int)::model::tetrimino::Move::LEFT );
					//mCurrentTetrimino.tryMoveLeft( mPlayerStage.cgrid( ) );
					it = eventQueue.erase( it );
					break;
				case sf::Keyboard::Right:
					input += TAG_TETRIMINO_MOVE + std::to_string( (int)::model::tetrimino::Move::RIGHT );
					//mCurrentTetrimino.tryMoveRight( mPlayerStage.cgrid( ) );
					it = eventQueue.erase( it );
					break;
				case sf::Keyboard::LShift:
					[[ fallthrough ]];
				case sf::Keyboard::Up:
					input += TAG_TETRIMINO_MOVE + std::to_string( (int)::model::tetrimino::Move::ROTATE );
					//mCurrentTetrimino.tryRotate( mPlayerStage.cgrid( ) );
					it = eventQueue.erase( it );
					break;
				/*case sf::Keyboard::Escape:
					if ( nullptr == mOverlappedScene_ ||
						typeid(*mOverlappedScene_) != typeid(::scene::inPlay::Assertion) )
					{
						retVal = ::scene::inPlay::ID::ASSERTION;
						it = eventQueue.erase( it );
					}
					else
					{
						++it;
					}
					break;*/
				default:
					++it;
					break;
			}
		}
		else
		{
			++it;
		}
	}

	mNet->send( input.data(), input.size() );
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
	if ( true == mHasCurrentTetrimino )
	{
		mCurrentTetrimino.draw( *mWindow_ );
	}
}