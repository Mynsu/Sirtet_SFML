#include "../pch.h"
#include "Stage.h"

const int8_t DEAD_LINE_NUMBER = 1;

void model::Stage::draw( sf::RenderWindow& window )
{
	window.draw( mPanel );
	for ( uint8_t i = 0; i != ::model::stage::GRID_HEIGHT; ++i )
	{
		for ( uint8_t k = 0; k != ::model::stage::GRID_WIDTH; ++k )
		{
			if ( true == mGrid[i][k].blocked )
			{
				mCellShape.setFillColor( mGrid[i][k].color );
				mCellShape.setPosition( mPosition_ + sf::Vector2f(k, i)*mCellSize_ );
				window.draw( mCellShape );
			}
		}
	}
}

bool model::Stage::isOver( ) const
{
	bool retVal = false;
	for ( const model::stage::Cell& cell : mGrid[DEAD_LINE_NUMBER] )
	{
		if ( true == cell.blocked )
		{
			retVal = true;
			break;
		}
	}
	return retVal;
}

void model::Stage::blackout( const sf::Color color )
{
	for ( auto& row : mGrid )
	{
		for ( model::stage::Cell& cell : row )
		{
			cell.color = color;
		}
	}
}

void model::Stage::clear( )
{
	for ( auto& row : mGrid )
	{
		for ( model::stage::Cell& cell : row )
		{
			cell.blocked = false;
		}
	}
}

uint8_t model::Stage::tryClearRow( )
{
	std::bitset<::model::stage::GRID_HEIGHT> fLinesCleared;
	// From the bottommost to the topmost,
	for ( int8_t i = ::model::stage::GRID_HEIGHT-1; i != -1; --i )
	{
		uint8_t k = 0;
		for ( ; k != ::model::stage::GRID_WIDTH; ++k )
		{
			// When the i-th line doesn't have less than one empty cell,
			if ( false == mGrid[i][k].blocked )
			{
				// Checks the upper line.
				break;
			}
		}
		// k reaches GRID_WIDTH when the i-th line is full with blocks.
		if ( ::model::stage::GRID_WIDTH == k )
		{
			for ( ::model::stage::Cell& cell : mGrid[i] )
			{
				// NOTE: Don't need to set it.color transparent.
				cell.blocked = false;
			}
			fLinesCleared.set( i );
			// When an I-shaped tetrimino is inserted vertical,
			const uint8_t MAX_CLEARED = ::model::tetrimino::BLOCKS_A_TETRIMINO;
			// There's no need to check after the i-th line.
			if ( MAX_CLEARED == fLinesCleared.count() )
			{
				break;
			}
		}
	}
	uint8_t numOflinesCleared = (uint8_t)fLinesCleared.count();
	// When One or more lines has been cleared,
	if ( 0 != numOflinesCleared )
	{
		// From the bottommost to the topmost,
		// NOTE: The topmost line can skip clearing check.
		for ( int8_t i = ::model::stage::GRID_HEIGHT-1; i != 0; --i )
		{
			if ( true == fLinesCleared.test(i) )
			{
				// the k-th line is the adjacent upper line of the i-th line.
				int8_t k = i-1;
				for ( ; k != -1; --k )
				{
					if ( false == fLinesCleared.test(k) )
					{
						mGrid[i] = mGrid[k];
						fLinesCleared.set( i, false );
						for ( ::model::stage::Cell& cell : mGrid[k] )
						{
							cell.blocked = false;
						}
						fLinesCleared.set( k );
						break;
					}
				}
				// That k is -1 means there's no more lines above, except the cleared lines.
				if ( INT8_MAX == k )
				{
					break;
				}
			}
		}
	}
	return numOflinesCleared;
}