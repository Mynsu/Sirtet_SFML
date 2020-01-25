#pragma once

namespace model
{
	class Stage
	{
	public:
		// NOTE: DO NOT USE.  Not deleted because std::unordered_map requires.
		Stage( )
			: mCellSize_( 0.f ), mWindow_( nullptr )
		{
		}
		Stage( sf::RenderWindow& window )
			: mCellSize_( 30.f ), mWindow_( &window )
		{
		}
		~Stage( ) = default;

		void deserialize( const ::model::stage::Grid* const ptr )
		{
			mGrid = *ptr;
		}
		uint8_t tryClearRow( );
		inline bool isOver( ) const
		{
			bool retVal = false;
			for ( const auto& it : mGrid[1] )
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
			for ( auto& row : mGrid )
			{
				for ( auto& cell : row )
				{
					// Gray
					cell.color = GRAY;
				}
			}
		}
		inline void clear( )
		{
			for ( auto& row : mGrid )
			{
				for ( auto& cell : row )
				{
					cell.blocked = false;
				}
			}
		}
		inline ::model::stage::Grid& grid( )
		{
			return mGrid;
		}
		inline const ::model::stage::Grid& cgrid( ) const
		{
			return mGrid;
		}
		inline sf::Vector2f position( ) const
		{
			return mPosition_;
		}
		inline void setPosition( const sf::Vector2f& position )
		{
			mPanel.setPosition( position );
			mPosition_ = position;
		}
		void setSize( const float cellSize );
		inline void setColor( const sf::Color background, const sf::Color cellOutlineColor )
		{
			mPanel.setFillColor( background );
			mCellShape.setOutlineThickness( 1.f );
			mCellShape.setOutlineColor( cellOutlineColor );
		}
		inline void setOutline( const float thickness, const sf::Color color )
		{
			mPanel.setOutlineThickness( thickness );
			mPanel.setOutlineColor( color );
		}
		inline void draw( )
		{
			mWindow_->draw( mPanel );
			// NOTE: The failure regarding OpenGL occurs.
			///#pragma omp parallel
			for ( uint8_t i = 0; i != ::model::stage::GRID_HEIGHT; ++i )
			{
				for ( uint8_t k = 0; k != ::model::stage::GRID_WIDTH; ++k )
				{
					if ( true == mGrid[i][k].blocked )
					{
						mCellShape.setFillColor( mGrid[i][k].color );
						mCellShape.setPosition( mPosition_ + sf::Vector2f(k, i)*mCellSize_ );
						mWindow_->draw( mCellShape );
					}
				}
			}
		}
	private:
		float mCellSize_;
		sf::RenderWindow* mWindow_;
		sf::Vector2f mPosition_;
		sf::RectangleShape mPanel, mCellShape;
		//0
		//1
		//o
		//o
		//o
		//19 == ::model::stage::GRID_HEIGHT
		::model::stage::Grid mGrid;
	};
}