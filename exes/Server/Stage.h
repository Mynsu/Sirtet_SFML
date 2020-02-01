#pragma once

namespace model
{
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
		inline void blackout( )
		{
			const sf::Color GRAY( 0x808080ff );
			for ( auto& row : mGrid )
			{
				for ( auto& cell : row )
				{
					// Gray
					cell.color = GRAY;
				}
			}
		}
		inline ::model::stage::Grid& grid( )
		{
			return mGrid;
		}
		inline const ::model::stage::Grid& cgrid( ) const
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
		::model::stage::Grid mGrid;
	};
}