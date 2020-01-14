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
			clearTetrimino( );
		}

		inline void setBackgroundColor( const sf::Color color,
									   const float outlineThickness, const sf::Color outlineColor,
									   const sf::Color cellOutlineColor )
		{
			mPanel.setFillColor( color );
			mPanel.setOutlineThickness( outlineThickness );
			mPanel.setOutlineColor( outlineColor );
			for ( sf::RectangleShape& block : mBlocks )
			{
				block.setOutlineThickness( 1.0f );
				block.setOutlineColor( cellOutlineColor );
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
			const sf::Vector2f margin( 10.f, 10.f );
			for ( sf::RectangleShape& block : mBlocks )
			{
				block.setPosition( position+margin );
				block.setSize( size );
			}
			mCellSize_ = cellSize;
		}
		void setTetrimino( const ::model::Tetrimino& next );
		void clearTetrimino( )
		{
			for ( sf::RectangleShape& block : mBlocks )
			{
				block.setFillColor( sf::Color::Transparent );
			}
		}
		void draw( );
	private:
		float mCellSize_;
		sf::Vector2f mPosition_;
		sf::RenderWindow& mWindow_;
		sf::RectangleShape mPanel;
		sf::RectangleShape mBlocks[::model::tetrimino::BLOCKS_A_TETRIMINO];
	};
}