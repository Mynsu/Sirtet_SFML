#include "pch.h"
#include <Common.h>
#include "Stage.h"

uint8_t model::Stage::tryClearRow( )
{
	std::bitset<::model::stage::GRID_HEIGHT> fLineCleared;
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
			fLineCleared.set( i );
			// When an I-shaped tetrimino is inserted vertical,
			const uint8_t MAX_CLEARED = ::model::tetrimino::BLOCKS_A_TETRIMINO;
			// There's no need to check after the i-th line.
			if ( MAX_CLEARED == fLineCleared.count( ) )
			{
				break;
			}
		}
	}
	uint8_t linesCleared = (uint8_t)fLineCleared.count();
	// When One or more lines has been cleared,
	if ( 0 != linesCleared )
	{
		// From the bottommost to the topmost,
		// NOTE: The topmost line can skip clearing check.
		for ( int8_t i = ::model::stage::GRID_HEIGHT-1; i != 0; --i )
		{
			if ( true == fLineCleared.test(i) )
			{
				// the k-th line is the adjacent upper line of the i-th line.
				int8_t k = i-1;
				for ( ; k != -1; --k )
				{
					if ( false == fLineCleared.test(k) )
					{
						mGrid[i] = mGrid[k];
						fLineCleared.set( i, false );
						for ( ::model::stage::Cell& cell : mGrid[k] )
						{
							cell.blocked = false;
						}
						fLineCleared.set( k );
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
	return linesCleared;
}

bool model::Stage::isOver( ) const
{
	bool retVal = false;
	for ( const ::model::stage::Cell& cell : mGrid[1] )
	{
		if ( true == cell.blocked )
		{
			retVal = true;
			break;
		}
	}
	return retVal;
}

void model::Stage::blackout( )
{
	const sf::Color GRAY( 0x808080ff );
	for ( auto& row : mGrid )
	{
		for ( ::model::stage::Cell& cell : row )
		{
			// Gray
			cell.color = GRAY;
		}
	}
}