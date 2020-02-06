#pragma once

namespace model
{
	class Stage
	{
	public:
		Stage( ) = default;
		virtual ~Stage( ) = default;

		uint8_t tryClearRow( );
		bool isOver( ) const;
		void blackout( );
		::model::stage::Grid& grid( )
		{
			return mGrid;
		}
		const ::model::stage::Grid& cgrid( ) const
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