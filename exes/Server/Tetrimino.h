#pragma once
#include <Lib/Common.h>

namespace model
{
	// 4 x 4 == BLOCKS_A_TETRIMINO x BLOCKS_A_TETRIMINO
	// e.g. 0b1000'1110'0000'0000 ==
	//		1000
	//		1110
	//		0000
	//		0000
	using LocalSpace = uint16_t;
	class Tetrimino
	{
	public:
		// Needs to be initialized by Spawn().
		Tetrimino( )
			: mIsFallingDown( false )
		{ }
		virtual ~Tetrimino( ) = default;

		static ::model::Tetrimino Spawn( );

		bool isFallingDown( ) const
		{
			return mIsFallingDown;
		}
		sf::Vector2<int8_t> position( ) const
		{
			return mPosition;
		}
		// Current blocks within their own local space.
		LocalSpace blocks( ) const
		{
			return mPossibleRotations[(int)mRotationID];
		}
		::model::tetrimino::Type type( ) const
		{
			return mType;
		}
		::model::tetrimino::Rotation rotationID( ) const
		{
			return mRotationID;
		}
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
		void fallDown( const bool isFallingDown = true )
		{
			mIsFallingDown = isFallingDown;
		}
	private:
		bool hasCollidedWith( const ::model::stage::Grid& grid ) const;
		bool mIsFallingDown;
		// X and y on global coordinate.
		// Unit: Grid.
		sf::Vector2<int8_t> mPosition;
		sf::Color mColor;
		::model::tetrimino::Type mType;
		::model::tetrimino::Rotation mRotationID;
		LocalSpace mPossibleRotations[(int)::model::tetrimino::Rotation::NONE_MAX];
		static sf::Vector2<int8_t> Test[(int)::model::tetrimino::Rotation::NONE_MAX][4];
	};
}