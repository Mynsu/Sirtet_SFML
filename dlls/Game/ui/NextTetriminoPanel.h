#pragma once
#include <Lib/Common.h>

namespace model
{
	class Tetrimino;
}

namespace ui
{
	class NextTetriminoPanel
	{
	public:
		NextTetriminoPanel( )
		{
			clearTetrimino( );
		}
		NextTetriminoPanel( const NextTetriminoPanel& ) = delete;
		void operator=( const NextTetriminoPanel& ) = delete;
		NextTetriminoPanel( NextTetriminoPanel&& ) = delete;
		virtual ~NextTetriminoPanel( ) = default;

		sf::FloatRect globalBounds( ) const
		{
			return mPanel.getGlobalBounds();
		}
		float rotation( ) const
		{
			return mPanel.getRotation();
		}
		void setColor( const sf::Color background, const sf::Color cellOutlineColor )
		{
			mPanel.setFillColor( background );
			for ( sf::RectangleShape& block : mBlocks )
			{
				block.setOutlineColor( cellOutlineColor );
			}
		}
		void setOutline( const float thickness, const sf::Color color )
		{
			mPanel.setOutlineThickness( thickness );
			mPanel.setOutlineColor( color );
		}
		void setDimension( const sf::Vector2f position, const float cellSize );
		void rotate( const float degree )
		{
			mPanel.rotate( degree );
		}
		void resetRotation( )
		{
			mPanel.setRotation( 0 );
		}
		void scale( const float factor )
		{
			mPanel.scale( factor, factor );
		}
		void resetScale( )
		{
			mPanel.setScale( 1.f, 1.f );
		}
		void setTetrimino( const ::model::Tetrimino& next );
		void clearTetrimino( )
		{
			for ( sf::RectangleShape& block : mBlocks )
			{
				block.setFillColor( sf::Color::Transparent );
				block.setOutlineThickness( 0.f );
			}
		}
		void draw( sf::RenderWindow& window );
	private:
		float mCellSize_;
		sf::Vector2f mLeftTopPosition;
		sf::RectangleShape mPanel;
		sf::RectangleShape mBlocks[::model::tetrimino::BLOCKS_A_TETRIMINO];
	};
}