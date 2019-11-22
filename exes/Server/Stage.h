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
		inline void lock( const uint8_t x, const uint8_t y )
		{
			ASSERT_TRUE( (x<::model::stage::GRID_WIDTH) && (y<::model::stage::GRID_HEIGHT) );
			mGrid[ y ][ x ].blocked = true;
		}
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
		inline const std::array< std::array<Cell, ::model::stage::GRID_WIDTH>, ::model::stage::GRID_HEIGHT >& grid( ) const
		{
			return mGrid;
		}
		inline std::array< std::array<Cell,::model::stage::GRID_WIDTH>, ::model::stage::GRID_HEIGHT >& floor( )
		{
			return mGrid;
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