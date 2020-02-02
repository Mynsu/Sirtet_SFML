#include "../pch.h"
#include "NextTetriminoPanel.h"
#include "../model/Tetrimino.h"

void ui::NextTetriminoPanel::setTetrimino( const::model::Tetrimino& next )
{
	const sf::Color nextTetColor( next.color() );
	for ( sf::RectangleShape& block : mBlocks )
	{
		block.setFillColor( nextTetColor );
		block.setOutlineThickness( 0.5f );
	}
	const ::model::tetrimino::Type type = next.type( );
	const ::model::tetrimino::LocalSpace nextTetBlocks = next.blocks( );
	const sf::Vector2f margin( mCellSize_, mCellSize_ );
	uint8_t k = 0;
	for ( uint8_t i = 0; i != ::model::tetrimino::BLOCKS_A_TETRIMINO*::model::tetrimino::BLOCKS_A_TETRIMINO; ++i )
	{
		if ( nextTetBlocks & (0x1<<(::model::tetrimino::BLOCKS_A_TETRIMINO*::model::tetrimino::BLOCKS_A_TETRIMINO-i-1)) )
		{
			sf::Vector2f localPos( sf::Vector2<uint8_t>(i%model::tetrimino::BLOCKS_A_TETRIMINO,i/model::tetrimino::BLOCKS_A_TETRIMINO) );
			switch( type )
			{
				case ::model::tetrimino::Type::I:
					mBlocks[k].setPosition( mLeftTopPosition + margin - sf::Vector2f(0.f, mCellSize_*0.5f)
										   + localPos*mCellSize_ );
					break;
				case ::model::tetrimino::Type::O:
					mBlocks[k].setPosition( mLeftTopPosition + margin
										   + localPos*mCellSize_ );
					break;
				default:
					mBlocks[k].setPosition( mLeftTopPosition + margin*1.5f + sf::Vector2f(0.f, mCellSize_*0.5f)
										   + localPos*mCellSize_ );
					break;
			}
			if ( ::model::tetrimino::BLOCKS_A_TETRIMINO == ++k )
			{
				break;
			}
		}
	}
}

void ui::NextTetriminoPanel::draw( )
{
	mWindow_.draw( mPanel );
	for ( auto& it : mBlocks )
	{
		mWindow_.draw( it );
	}
}

void ui::NextTetriminoPanel::setDimension( const sf::Vector2f position, const float cellSize)
{
	const uint8_t side = ::model::tetrimino::BLOCKS_A_TETRIMINO+2;
	sf::Vector2f size( side, side );
	size *= cellSize;
	mPanel.setSize( size );
	size *= 0.5f;
	mPanel.setOrigin( size );
	mPanel.setPosition( position );
	mLeftTopPosition = position-size;
	size = sf::Vector2f(cellSize, cellSize);
	for ( sf::RectangleShape& block : mBlocks )
	{
		block.setPosition( position );
		block.setSize( size );
	}
	mCellSize_ = cellSize;
}
