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
			const uint8_t side = ::model::tetrimino::BLOCKS_A_TETRIMINO+2;
			sf::Vector2f size( side, side );
			size *= cellSize;
			mPanel.setSize( size );
			size *= 0.5f;
			mPanel.setOrigin( size );
			mPanel.setPosition( position );
			mLeftTopPosition = position-size;
			size = sf::Vector2f(cellSize, cellSize);
			for ( sf::RectangleShape& block : mBlocks )
			{
				block.setPosition( position );
				block.setSize( size );
			}
			mCellSize_ = cellSize;
		}
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