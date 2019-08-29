#include "Stage.h"
#include <bitset>

uint8_t model::Stage::clearLine( )
{
	std::bitset<::model::stage::GRID_HEIGHT> fLineCleared;
	for ( int8_t i = ::model::stage::GRID_HEIGHT-1; i != -1; --i )
	{
		uint8_t k = 0u;
		for ( ; k != ::model::stage::GRID_WIDTH; ++k )
		{
			if ( false == mGrid[ i ][ k ].blocked )
			{
				break;
			}
		}
		if ( ::model::stage::GRID_WIDTH == k )
		{
			for ( auto& it : mGrid[ i ] )
			{
				it.blocked = false;
			}
			fLineCleared.set( i );
			if ( 4 == fLineCleared.count( ) ) //궁금: 변수 따로 놓는 게 빠르려나?
			{
				break;
			}
		}
	}
	uint8_t linesCleared = static_cast<uint8_t>(fLineCleared.count( ));
	if ( 0u != linesCleared )
	{
		for ( int8_t i = ::model::stage::GRID_HEIGHT-1; i != 0; --i )//궁금: 이거 완죤... 정렬 아니냐? //TODO: -1 아닌 거 거슬리네.
		{
			if ( true == fLineCleared.test( i ) )
			{
				int8_t k = i-1;
				for ( ; k != -1; --k )
				{
					if ( false == fLineCleared.test( k ) )
					{
						mGrid[ i ] = mGrid[ k ]; //TODO: 리스트가 낫겠다.
						fLineCleared.set( i, false );
						for ( auto& it : mGrid[ k ] )
						{
							it.blocked = false;
						}
						fLineCleared.set( k );
						break;
					}
				}
				if ( -1 == k )
				{
					break;
				}
			}
		}
	}
	return linesCleared;
}

void model::Stage::setSize( const float cellSize )
{
	ASSERT_TRUE( 0 < cellSize );
	mPanel.setSize( sf::Vector2f(::model::stage::GRID_WIDTH,::model::stage::GRID_HEIGHT) * cellSize );
	mCellShape.setSize( sf::Vector2f( cellSize, cellSize ) );
	mCellSize_ = cellSize;
}
