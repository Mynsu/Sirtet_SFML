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
		inline Tetrimino( )
			: mIsFallingDown( false )
		{
			mBlockShape.setOutlineThickness( 1.f );
			mBlockShape.setOutlineColor( sf::Color::Black );
		}
		inline Tetrimino( const Tetrimino& arg )
			: mIsFallingDown( false ), mType( arg.mType ),
			mRotationID( arg.mRotationID ), mPosition( arg.mPosition )
		{
			mBlockShape.setFillColor( arg.mBlockShape.getFillColor( ) );
			mBlockShape.setOutlineThickness( 1.f );
			mBlockShape.setOutlineColor( sf::Color::Black );
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
			mBlockShape.setFillColor( arg.mBlockShape.getFillColor( ) );
			for ( uint8_t i = 0u; i != (uint8_t)::model::tetrimino::Rotation::NULL_MAX; ++i )
			{
				mPossibleRotations[ i ] = arg.mPossibleRotations[ i ];
			}
		}
		~Tetrimino( ) = default;

		static Tetrimino Spawn( );

		inline void draw( sf::RenderWindow& window )
		{
			// NOTE: The OpenGL failure occurs.
			///#pragma omp parallel
			for ( uint8_t i = 0; i != ::model::tetrimino::LOCAL_SPACE_SIZE; ++i )
			{
				if ( mPossibleRotations[(int)mRotationID] &
					(0x1u<<(::model::tetrimino::LOCAL_SPACE_SIZE-i-1u)) )
				{
					// Coordinate transformation
					const sf::Vector2< int8_t > localPos( i%model::tetrimino::BLOCKS_A_TETRIMINO, i/model::tetrimino::BLOCKS_A_TETRIMINO );
					mBlockShape.setPosition( mOrigin_ + sf::Vector2f(mPosition+localPos)*mBlockSize_ );
					window.draw( mBlockShape );
				}
			}
		}
		inline bool isFallingDown( ) const
		{
			return mIsFallingDown;
		}
		inline sf::Vector2<int8_t> position( ) const
		{
			return mPosition;
		}
		inline sf::Color color( ) const
		{
			return mBlockShape.getFillColor( );
		}
		// Current blocks within their own local space.
		inline ::model::tetrimino::LocalSpace blocks( ) const
		{
			return mPossibleRotations[ static_cast<int>(mRotationID) ];
		}
		inline ::model::tetrimino::Type type( ) const
		{
			return mType;
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
		void land( std::array< std::array<::model::Cell,::model::stage::GRID_WIDTH>, ::model::stage::GRID_HEIGHT >& grid );
		inline void fallDown( const bool isFallingDown = true )
		{
			mIsFallingDown = isFallingDown;
		}
		inline void setInfo( const ::model::tetrimino::Info& info )
		{
			mPosition = info.position;
			setType( info.type );
			///mRotationID = info.rotationID;
		}
		inline void setOrigin( const sf::Vector2f& origin )
		{
			mOrigin_ = origin;
		}
		inline void setSize( const float blockSize )
		{
			ASSERT_TRUE( 0 < blockSize );
			mBlockShape.setSize( sf::Vector2f( blockSize, blockSize ) );
			mBlockSize_ = blockSize;
		}
		inline void setColor( const sf::Color color )
		{
			mBlockShape.setFillColor( color );
		}
	private:
		bool hasCollidedWith( const std::array< std::array<Cell,::model::stage::GRID_WIDTH>, ::model::stage::GRID_HEIGHT >& grid ) const;
		void setType( const ::model::tetrimino::Type type );
		
		bool mIsFallingDown;
		// X and y on global coordinate.
		// Unit: Grid.
		sf::Vector2< int8_t > mPosition;
		::model::tetrimino::Type mType;
		::model::tetrimino::Rotation mRotationID;
		float mBlockSize_;
		sf::Vector2f mOrigin_;
		sf::RectangleShape mBlockShape;
		::model::tetrimino::LocalSpace mPossibleRotations[ (int)::model::tetrimino::Rotation::NULL_MAX ];
		static sf::Vector2<int8_t> Test[ (int)::model::tetrimino::Rotation::NULL_MAX ][ 4 ];
	};
}