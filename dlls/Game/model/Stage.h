#pragma once
#include "../Constant.h"

namespace model
{
	// A stage consists of cells as a tetrimino consists of blocks.
	// Strictly, cell isn't block and vice versa, but they match up each other.
	struct Cell
	{
		inline Cell( )
			: blocked( false ), color( sf::Color::Transparent )
		{ }
		bool blocked;
		sf::Color color;
	};

	class Stage
	{
	public:
		Stage( ) = delete;
		explicit Stage( sf::RenderWindow& window )
			: mCellSize_( 30.f ), mWindow_( window )
		{
			mPanel.setFillColor( sf::Color::Black );
			mCellShape.setOutlineThickness( 1.f );
			mCellShape.setOutlineColor( sf::Color::Black );
		}
		~Stage( ) = default;

		inline void draw( )
		{
			mWindow_.draw( mPanel );
			// NOTE: The failure regarding OpenGL occurs.
			///#pragma omp parallel
			for ( uint8_t i = 0u; i != ::model::stage::GRID_HEIGHT; ++i )
			{
				for ( uint8_t k = 0u; k != ::model::stage::GRID_WIDTH; ++k )
				{
					if ( true == mGrid[ i ][ k ].blocked )
					{
						mCellShape.setFillColor( mGrid[ i ][ k ].color );
						mCellShape.setPosition( mPosition_ + sf::Vector2f(static_cast<float>(k), static_cast<float>(i))*mCellSize_ );
						mWindow_.draw( mCellShape );
					}
				}
			}
		}
		uint8_t tryClearRow( );
		inline sf::Vector2f position( ) const
		{
			return mPosition_;
		}
		inline void lock( const uint8_t x, const uint8_t y, const sf::Color color )
		{
			ASSERT_TRUE( (x<::model::stage::GRID_WIDTH) && (y<::model::stage::GRID_HEIGHT) );
			mGrid[ y ][ x ].blocked = true;
			mGrid[ y ][ x ].color = color;
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
		inline void blackout( )
		{
			const sf::Color GRAY( 0x808080ff );
			#pragma omp parallel
			for ( auto& it : mGrid )
			{
				for ( auto& itit : it )
				{
					// Gray
					itit.color = GRAY;
				}
			}
		}
		inline const std::array< std::array<Cell,::model::stage::GRID_WIDTH>, ::model::stage::GRID_HEIGHT >& grid( ) const
		{
			return mGrid;
		}
		inline void setPosition( const sf::Vector2f& position )
		{
			mPanel.setPosition( position );
			mPosition_ = position;
		}
		void setSize( const float cellSize );
	private:
		float mCellSize_;
		sf::RenderWindow& mWindow_;
		sf::Vector2f mPosition_;
		sf::RectangleShape mPanel, mCellShape;
		//0
		//1
		//o
		//o
		//o
		//19 == ::model::stage::GRID_HEIGHT
		std::array< std::array<Cell,::model::stage::GRID_WIDTH>, ::model::stage::GRID_HEIGHT > mGrid;
	};
}