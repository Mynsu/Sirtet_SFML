#pragma once

namespace model
{
	class Tetrimino;
}

namespace ui
{
	class NextTetriminoPanel
	{
	public:
		inline NextTetriminoPanel( sf::RenderWindow& window )
			: mWindow_( window )
		{
		}

		inline void setBackgroundColor( const sf::Color color,
									   const float outlineThickness, const sf::Color outlineColor,
									   const sf::Color cellOutlineColor )
		{
			mPanel.setFillColor( color );
			mPanel.setOutlineThickness( outlineThickness );
			mPanel.setOutlineColor( outlineColor );
			for ( uint8_t i = 0; ::model::tetrimino::BLOCKS_A_TETRIMINO != i; ++i )
			{
				mBlocks[i].setOutlineThickness( 1.0f );
				mBlocks[i].setOutlineColor( cellOutlineColor );
			}
		}
		inline void setDimension( const sf::Vector2f position, const float cellSize )
		{
			mPanel.setPosition( position );
			mPosition_ = position;
			mPanel.setSize(
				sf::Vector2f(::model::tetrimino::BLOCKS_A_TETRIMINO+2,
							 ::model::tetrimino::BLOCKS_A_TETRIMINO+2)*cellSize );
			const sf::Vector2f size(cellSize, cellSize);
			for ( uint8_t i = 0; ::model::tetrimino::BLOCKS_A_TETRIMINO != i; ++i )
			{
				mBlocks[i].setSize( size );
			}
			mCellSize_ = cellSize;
		}
		void setTetrimino( const ::model::Tetrimino& next );
		void draw( );
	private:
		float mCellSize_;
		sf::Vector2f mPosition_;
		sf::RenderWindow& mWindow_;
		sf::RectangleShape mPanel;
		sf::RectangleShape mBlocks[::model::tetrimino::BLOCKS_A_TETRIMINO];
	};
}