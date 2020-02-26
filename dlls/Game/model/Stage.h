#pragma once
#include <Common.h>

namespace model
{
	class Stage
	{
	public:
		Stage( )
			: mCellSize_( 30.f )
		{}
		virtual ~Stage( ) = default;

		void deserialize( const ::model::stage::Grid& grid )
		{
			mGrid = grid;
		}
		uint8_t tryClearRow( );
		bool isOver( ) const;
		void blackout( const sf::Color color );
		void clear( );
		::model::stage::Grid& grid( )
		{
			return mGrid;
		}
		const ::model::stage::Grid& cgrid( ) const
		{
			return mGrid;
		}
		sf::Vector2f position( ) const
		{
			return mPosition_;
		}
		void setPosition( const sf::Vector2f position )
		{
			mPanel.setPosition( position );
			mPosition_ = position;
		}
		void setSize( const float cellSize )
		{
			ASSERT_TRUE( 0.f < cellSize );
			mPanel.setSize( sf::Vector2f(::model::stage::GRID_WIDTH*cellSize,
										 ::model::stage::GRID_HEIGHT*cellSize)  );
			mCellShape.setSize( sf::Vector2f(cellSize, cellSize) );
			mCellSize_ = cellSize;
		}
		void setColor( const sf::Color background, const sf::Color cellOutlineColor )
		{
			mPanel.setFillColor( background );
			mCellShape.setOutlineThickness( -2.f );
			mCellShape.setOutlineColor( cellOutlineColor );
		}
		void setOutline( const float thickness, const sf::Color color )
		{
			mPanel.setOutlineThickness( thickness );
			mPanel.setOutlineColor( color );
		}
		void draw( sf::RenderWindow& window );
	private:
		// Can be figured out from mCellShape.
		float mCellSize_;
		// Can be earned from mPanel.
		sf::Vector2f mPosition_;
		sf::RectangleShape mPanel, mCellShape;
		//0
		//1
		//.
		//.
		//.
		//19 == ::model::stage::GRID_HEIGHT
		::model::stage::Grid mGrid;
	};
}