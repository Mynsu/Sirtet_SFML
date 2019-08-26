#include "Stage.h"

void model::Stage::setSize( const float cellSize )
{
	ASSERT_TRUE( 0 < cellSize );
	mPanel.setSize( sf::Vector2f(::model::stage::GRID_WIDTH,::model::stage::GRID_HEIGHT) * cellSize );
	mCellShape.setSize( sf::Vector2f( cellSize, cellSize ) );
	mCellSize_ = cellSize;
}
