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

		inline sf::FloatRect globalBounds( ) const
		{
			return mPanel.getGlobalBounds();
		}
		inline float rotation( ) const
		{
			return mPanel.getRotation();
		}
		inline void setColor( const sf::Color background, const sf::Color cellOutlineColor )
		{
			mPanel.setFillColor( background );
			for ( sf::RectangleShape& block : mBlocks )
			{
				block.setOutlineThickness( 1.0f );
				block.setOutlineColor( cellOutlineColor );
			}
		}
		inline void setOutline( const float thickness, const sf::Color color )
		{
			mPanel.setOutlineThickness( thickness );
			mPanel.setOutlineColor( color );
		}
		void setDimension( const sf::Vector2f position, const float cellSize );
		inline void rotate( const float degree )
		{
			mPanel.rotate( degree );
		}
		inline void resetRotation( )
		{
			mPanel.setRotation( 0 );
		}
		inline void scale( const float factor )
		{
			mPanel.scale( factor, factor );
		}
		inline void resetScale( )
		{
			mPanel.setScale( 1.f, 1.f );
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
		sf::Vector2f mLeftTopPosition;
		sf::RenderWindow& mWindow_;
		sf::RectangleShape mPanel;
		sf::RectangleShape mBlocks[::model::tetrimino::BLOCKS_A_TETRIMINO];
	};
}