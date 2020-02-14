#pragma once
#include "Stage.h"

namespace model
{
	// 4 x 4 == BLOCKS_A_TETRIMINO x BLOCKS_A_TETRIMINO
	// e.g. 0b1000'1110'0000'0000 ==
	//		1000
	//		1110
	//		0000
	//		0000
	class Tetrimino
	{
	public:
		// Needs initialization.
		Tetrimino( );
		Tetrimino( const Tetrimino& arg );
		void operator=( const Tetrimino& arg );
		virtual ~Tetrimino( ) = default;

		static void LoadResources( );
		static Tetrimino Spawn( ::model::tetrimino::Type type = ::model::tetrimino::Type::NONE_MAX );
		
		// Returns true when colliding with the floor or another tetrimino.
		bool moveDown( const ::model::stage::Grid& grid, const uint8_t diff = 1 )
		{
			ASSERT_TRUE( diff < ::model::stage::GRID_HEIGHT );
			mPosition.y += diff;
			const bool retVal = hasCollidedWith(grid);
			if ( true == retVal )
			{
				mPosition.y -= diff;
			}
			return retVal;
		}
		void tryMoveLeft( const ::model::stage::Grid& grid, const uint8_t diff = 1 )
		{
			ASSERT_TRUE( diff < ::model::stage::GRID_WIDTH );
			Tetrimino afterMove( *this );
			afterMove.mPosition.x -= diff;
			if ( false == afterMove.hasCollidedWith( grid ) )
			{
				*this = afterMove;
			}
		}
		void tryMoveRight( const ::model::stage::Grid& grid, const uint8_t diff = 1 )
		{
			ASSERT_TRUE( diff < ::model::stage::GRID_WIDTH );
			Tetrimino afterMove( *this );
			afterMove.mPosition.x += diff;
			if ( false == afterMove.hasCollidedWith( grid ) )
			{
				*this = afterMove;
			}
		}
		// Rotates counter-clockwise.
		void tryRotate( const ::model::stage::Grid& grid );
		void land( ::model::stage::Grid& grid );
		void move( const ::model::tetrimino::Rotation rotationID, const sf::Vector2<int8_t> position )
		{
			mRotationID = rotationID;
			mPosition = position;
		}
		void setOrigin( const sf::Vector2f& origin )
		{
			mOrigin_ = origin;
		}
		sf::Vector2<int8_t> position( ) const
		{
			return mPosition;
		}
		::model::tetrimino::Type type( ) const
		{
			return mType;
		}
		sf::Color color( ) const
		{
			return mBlockShape.getFillColor( );
		}
		void setColor( const sf::Color color, const sf::Color outlineColor )
		{
			mBlockShape.setFillColor( color );
			mBlockShape.setOutlineColor( outlineColor );
		}
		void setSize( const float blockSize )
		{
			ASSERT_TRUE( 0.f < blockSize );
			mBlockShape.setSize( sf::Vector2f( blockSize, blockSize ) );
			mBlockSize_ = blockSize;
		}
		// Current blocks within their own local space.
		::model::tetrimino::LocalSpace blocks( ) const
		{
			return mPossibleRotations[(int)mRotationID];
		}
		bool isFallingDown( ) const
		{
			return mIsFallingDown;
		}
		void fallDown( const bool isFallingDown = true )
		{
			mIsFallingDown = isFallingDown;
		}

		void draw( sf::RenderWindow& window )
		{
			for ( uint8_t i = 0; i != ::model::tetrimino::LOCAL_SPACE_SIZE; ++i )
			{
				if ( mPossibleRotations[(int)mRotationID] &
					(0x1u<<(::model::tetrimino::LOCAL_SPACE_SIZE-i-1)) )
				{
					// Coordinate transformation
					const sf::Vector2<int8_t> localPos( i%model::tetrimino::BLOCKS_A_TETRIMINO,
														i/model::tetrimino::BLOCKS_A_TETRIMINO );
					mBlockShape.setPosition( mOrigin_ + sf::Vector2f(mPosition+localPos)*mBlockSize_ );
					window.draw( mBlockShape );
				}
			}
		}
	private:
		bool hasCollidedWith( const ::model::stage::Grid& grid ) const;
		bool mIsFallingDown;
		// X and y on global coordinate.
		// Unit: Grid.
		sf::Vector2<int8_t> mPosition;
		::model::tetrimino::Type mType;
		::model::tetrimino::Rotation mRotationID;
		float mBlockSize_;
		sf::Vector2f mOrigin_;
		sf::RectangleShape mBlockShape;
		::model::tetrimino::LocalSpace mPossibleRotations[(int)::model::tetrimino::Rotation::NONE_MAX];
		static sf::Vector2<int8_t> Test[(int)::model::tetrimino::Rotation::NONE_MAX][4];
		static sf::Color OutlineColor;
		static sf::Color Colors[(int)::model::tetrimino::Type::NONE_MAX];
	};
}