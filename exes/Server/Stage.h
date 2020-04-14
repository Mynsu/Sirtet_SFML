#pragma once

namespace model
{
	class Stage
	{
	public:
		Stage( )
		{
			ZeroMemory(&mGrid, sizeof(mGrid));
		}
		Stage( const Stage&	) = delete;
		Stage( Stage&& ) = delete;
		void operator=( const Stage& ) = delete;
		virtual ~Stage( ) = default;

		// Returns how many lines has been cleared.
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