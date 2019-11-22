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
	using LocalSpace = uint16_t;
	class Tetrimino
	{
	public:
		// Needs initialization.
		inline Tetrimino( )
			: mIsFallingDown( false )
		{
		}
		inline Tetrimino( const Tetrimino& arg )
			: mIsFallingDown( false ), mType( arg.mType ),
			mRotationID( arg.mRotationID ), mPosition( arg.mPosition )
		{
			for ( uint8_t i = 0u; i != (uint8_t)::model::tetrimino::Rotation::NULL_MAX; ++i )
			{
				mPossibleRotations[ i ] = arg.mPossibleRotations[ i ];
			}
		}
		inline void operator=( const Tetrimino& arg )
		{
			mType = arg.mType;
			mRotationID = arg.mRotationID;
			mPosition = arg.mPosition;
			for ( uint8_t i = 0u; i != (uint8_t)::model::tetrimino::Rotation::NULL_MAX; ++i )
			{
				mPossibleRotations[ i ] = arg.mPossibleRotations[ i ];
			}
		}
		~Tetrimino( ) = default;

		static Tetrimino Spawn( );

		inline bool isFallingDown( ) const
		{
			return mIsFallingDown;
		}
		inline sf::Vector2<int8_t> position( ) const
		{
			return mPosition;
		}
		// Current blocks within their own local space.
		inline LocalSpace blocks( ) const
		{
			return mPossibleRotations[ static_cast<int>(mRotationID) ];
		}
		inline ::model::tetrimino::Type type( ) const
		{
			return mType;
		}
		inline ::model::tetrimino::Rotation rotationID( ) const
		{
			return mRotationID;
		}
		// Returns true when colliding with the floor or another tetrimino.
		inline bool moveDown( const std::array< std::array<::model::Cell,::model::stage::GRID_WIDTH>, ::model::stage::GRID_HEIGHT >& grid, const uint8_t diff = 1u )
		{
			ASSERT_TRUE( diff < ::model::stage::GRID_HEIGHT );
			mPosition.y += diff;
			return hasCollidedWith( grid );
		}
		inline void tryMoveLeft( const std::array< std::array<::model::Cell,::model::stage::GRID_WIDTH>, ::model::stage::GRID_HEIGHT >& grid, const uint8_t diff = 1u )
		{
			ASSERT_TRUE( diff < ::model::stage::GRID_WIDTH );
			Tetrimino afterMove( *this );
			afterMove.mPosition.x -= diff;
			if ( false == afterMove.hasCollidedWith( grid ) )
			{
				*this = afterMove;
			}
		}
		inline void tryMoveRight( const std::array< std::array<::model::Cell,::model::stage::GRID_WIDTH>, ::model::stage::GRID_HEIGHT >& grid, const uint8_t diff = 1u )
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
		void tryRotate( const std::array< std::array<::model::Cell,::model::stage::GRID_WIDTH>, ::model::stage::GRID_HEIGHT >& grid );
		void land( std::array< std::array<::model::Cell,::model::stage::GRID_WIDTH>, ::model::stage::GRID_HEIGHT >& floor );
		inline void fallDown( const bool isFallingDown = true )
		{
			mIsFallingDown = isFallingDown;
		}
	private:
		bool hasCollidedWith( const std::array< std::array<Cell,::model::stage::GRID_WIDTH>, ::model::stage::GRID_HEIGHT >& grid ) const;
		bool mIsFallingDown;
		// X and y on global coordinate.
		// Unit: Grid.
		sf::Vector2< int8_t > mPosition;
		::model::tetrimino::Type mType;
		::model::tetrimino::Rotation mRotationID;
		LocalSpace mPossibleRotations[ (int)::model::tetrimino::Rotation::NULL_MAX ];
		static sf::Vector2<int8_t> Test[ (int)::model::tetrimino::Rotation::NULL_MAX ][ 4 ];
	};
}