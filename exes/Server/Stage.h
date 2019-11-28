#pragma once

namespace model
{
	// A stage consists of cells as a tetrimino consists of blocks.
	// Strictly, cell isn't block and vice versa, but they match up each other.
	struct Cell
	{
		inline Cell( )
			: blocked( false )
		{
		}
		bool blocked;
	};

	class Stage
	{
	public:
		Stage( ) = default;
		~Stage( ) = default;

		uint8_t tryClearRow( );
		inline bool isOver( ) const
		{
			bool retVal = false;
			for ( const auto& it : mGrid[ 1 ] )
			{
				if ( true == it.blocked )
				{
					retVal = true;
					break;
				}
			}
			return retVal;
		}
		inline auto grid( ) -> std::array< std::array<Cell, ::model::stage::GRID_WIDTH>, ::model::stage::GRID_HEIGHT >&
		{
			return mGrid;
		}
		inline auto pGrid( ) -> std::array< std::array<Cell, ::model::stage::GRID_WIDTH>, ::model::stage::GRID_HEIGHT >*
		{
			return &mGrid;
		}
	private:
		//0
		//1
		//o
		//o
		//o
		//19 == ::model::stage::GRID_HEIGHT
		std::array< std::array<Cell, ::model::stage::GRID_WIDTH>, ::model::stage::GRID_HEIGHT > mGrid;
	};
}