#pragma once
#pragma hdrstop
#include <Lib/precompiled.h>
#include <array>
#include "../Constant.h"

namespace model
{
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
			for ( uint8_t i = 0u; i != ::model::stage::GRID_HEIGHT; ++i )
			{
				for ( uint8_t k = 0u; k != ::model::stage::GRID_WIDTH; ++k )
				{
					if ( true == mGrid[ i ][ k ].blocked )
					{
						mCellShape.setFillColor( mGrid[ i ][ k ].color );
						mCellShape.setPosition( mOrigin_ + sf::Vector2f(static_cast<float>(k), static_cast<float>(i))*mCellSize_ );
						mWindow_.draw( mCellShape );
					}
				}
			}
		}
		uint8_t clearLine( );
		inline const std::array< std::array<Cell,::model::stage::GRID_WIDTH>, ::model::stage::GRID_HEIGHT >& grid( ) const
		{
			return mGrid;
		}
		inline void setPosition( const sf::Vector2f& position )
		{
			mPanel.setPosition( position );
			mOrigin_ = position;
		}
		void setSize( const float cellSize );
		inline void lock( const uint8_t x, const uint8_t y, const sf::Color color )
		{
			ASSERT_TRUE( (x<::model::stage::GRID_WIDTH) && (y<::model::stage::GRID_HEIGHT) );
			mGrid[ y ][ x ].blocked = true;
			mGrid[ y ][ x ].color = color;
		}
	private:
		float mCellSize_;
		sf::RenderWindow& mWindow_;
		sf::Vector2f mOrigin_;
		sf::RectangleShape mPanel, mCellShape;
		std::array< std::array<Cell,::model::stage::GRID_WIDTH>, ::model::stage::GRID_HEIGHT > mGrid;
	};
}