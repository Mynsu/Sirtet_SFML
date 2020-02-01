#include "pch.h"
#include "Stage.h"

uint8_t model::Stage::tryClearRow( )
{
	std::bitset<::model::stage::GRID_HEIGHT> fLineCleared;
	// From the bottommost to the topmost,
	for ( int8_t i = ::model::stage::GRID_HEIGHT-1; i != -1; --i )
	{
		uint8_t k = 0u;
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
			for ( auto& it : mGrid[ i ] )
			{
				// NOTE: Don't need to set it.color transparent.
				it.blocked = false;
			}
			fLineCleared.set( i );
			// When an I-shaped tetrimino is inserted vertical,
			const uint8_t MAX_CLEARED = 4u;
			// There's no need to check after the i-th line.
			if ( MAX_CLEARED == fLineCleared.count( ) )
			{
				break;
			}
		}
	}
	uint8_t linesCleared = static_cast<uint8_t>(fLineCleared.count( ));
	// When One or more lines has been cleared,
	if ( 0u != linesCleared )
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
						mGrid[ i ] = mGrid[ k ];
						fLineCleared.set( i, false );
						for ( auto& it : mGrid[ k ] )
						{
							it.blocked = false;
						}
						fLineCleared.set( k );
						break;
					}
				}
				// That k is -1 means there's no more lines above, except the cleared lines.
				if ( -1 == k )
				{
					break;
				}
			}
		}
	}
	return linesCleared;
}