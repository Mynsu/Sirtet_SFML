#include "Stage.h"

void model::Stage::clearLine( )
{
	int8_t start = -1;
	int8_t end = -1;
	for ( int8_t i = ::model::stage::GRID_HEIGHT - 1; i != -1; --i )
	{
		uint8_t k = 0u;
		for ( ; k != ::model::stage::GRID_WIDTH; ++k )
		{
			if ( false == mGrid[ i ][ k ].blocked )
			{
				if ( -1 == start )
				{
					break;
				}
				else
				{
					goto breakbreak;
				}
			}
		}
		if ( ::model::stage::GRID_WIDTH == k )
		{
			if ( -1 == start )
			{
				start = i;
				end = i;
			}
			else
			{
				end = i;
			}
		}
	}
breakbreak:
	if ( -1 != start )
	{
		for ( uint8_t i = 0u; i != 2*end-start-1; ++i )
		{
			mGrid[ start - i ] = mGrid[ end - i - 1u ];
		}
		for ( int8_t i = start - end; i != -1; --i )
		{
			for ( auto& it : mGrid[ i ] )
			{
				it.blocked = false;
				it.color = sf::Color::Transparent;
			}
		}
	}
}

void model::Stage::setSize( const float cellSize )
{
	ASSERT_TRUE( 0 < cellSize );
	mPanel.setSize( sf::Vector2f(::model::stage::GRID_WIDTH,::model::stage::GRID_HEIGHT) * cellSize );
	mCellShape.setSize( sf::Vector2f( cellSize, cellSize ) );
	mCellSize_ = cellSize;
}
