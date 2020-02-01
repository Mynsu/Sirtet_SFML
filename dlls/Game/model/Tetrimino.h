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
		}
		inline Tetrimino( const Tetrimino& arg )
			: mIsFallingDown( false ), mType( arg.mType ),
			mRotationID( arg.mRotationID ), mPosition( arg.mPosition )
		{
			mBlockShape.setFillColor( arg.mBlockShape.getFillColor() );
			mBlockShape.setOutlineThickness( 1.f );
			mBlockShape.setOutlineColor( arg.mBlockShape.getOutlineColor() );
			for ( uint8_t i = 0u; i != (uint8_t)::model::tetrimino::Rotation::NONE_MAX; ++i )
			{
				mPossibleRotations[ i ] = arg.mPossibleRotations[ i ];
			}
		}
		inline void operator=( const Tetrimino& arg )
		{
			mType = arg.mType;
			mRotationID = arg.mRotationID;
			mPosition = arg.mPosition;
			mBlockShape.setFillColor( arg.mBlockShape.getFillColor() );
			mBlockShape.setOutlineColor( arg.mBlockShape.getOutlineColor() );
			for ( uint8_t i = 0u; i != (uint8_t)::model::tetrimino::Rotation::NONE_MAX; ++i )
			{
				mPossibleRotations[ i ] = arg.mPossibleRotations[ i ];
			}
		}
		~Tetrimino( ) = default;

		static void LoadResources( );
		static Tetrimino Spawn( ::model::tetrimino::Type type = ::model::tetrimino::Type::NONE_MAX );
		
		// Returns true when colliding with the floor or another tetrimino.
		inline bool moveDown( const ::model::stage::Grid& grid, const uint8_t diff = 1 )
		{
			ASSERT_TRUE( diff < ::model::stage::GRID_HEIGHT );
			mPosition.y += diff;
			const bool retVal = hasCollidedWith( grid );
			if ( true == retVal )
			{
				mPosition.y -= diff;
			}
			return retVal;
		}
		inline void tryMoveLeft( const ::model::stage::Grid& grid, const uint8_t diff = 1 )
		{
			ASSERT_TRUE( diff < ::model::stage::GRID_WIDTH );
			Tetrimino afterMove( *this );
			afterMove.mPosition.x -= diff;
			if ( false == afterMove.hasCollidedWith( grid ) )
			{
				*this = afterMove;
			}
		}
		inline void tryMoveRight( const ::model::stage::Grid& grid, const uint8_t diff = 1u )
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
		inline void move( const ::model::tetrimino::Rotation rotationID, const sf::Vector2<int8_t> position )
		{
			mRotationID = rotationID;
			mPosition = position;
		}
		inline void setOrigin( const sf::Vector2f& origin )
		{
			mOrigin_ = origin;
		}
		inline sf::Vector2<int8_t> position( ) const
		{
			return mPosition;
		}
		inline ::model::tetrimino::Type type( ) const
		{
			return mType;
		}
		inline sf::Color color( ) const
		{
			return mBlockShape.getFillColor( );
		}
		inline void setColor( const sf::Color color, const sf::Color outlineColor )
		{
			mBlockShape.setFillColor( color );
			mBlockShape.setOutlineColor( outlineColor );
		}
		inline void setSize( const float blockSize )
		{
			ASSERT_TRUE( 0 < blockSize );
			mBlockShape.setSize( sf::Vector2f( blockSize, blockSize ) );
			mBlockSize_ = blockSize;
		}
		// Current blocks within their own local space.
		inline ::model::tetrimino::LocalSpace blocks( ) const
		{
			return mPossibleRotations[ static_cast<int>(mRotationID) ];
		}
		inline bool isFallingDown( ) const
		{
			return mIsFallingDown;
		}
		inline void fallDown( const bool isFallingDown = true )
		{
			mIsFallingDown = isFallingDown;
		}

		inline void draw( sf::RenderWindow& window )
		{
			for ( uint8_t i = 0; i != ::model::tetrimino::LOCAL_SPACE_SIZE; ++i )
			{
				if ( mPossibleRotations[(int)mRotationID] &
					(0x1u<<(::model::tetrimino::LOCAL_SPACE_SIZE-i-1u)) )
				{
					// Coordinate transformation
					const sf::Vector2< int8_t > localPos( i%model::tetrimino::BLOCKS_A_TETRIMINO,
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
		sf::Vector2< int8_t > mPosition;
		::model::tetrimino::Type mType;
		::model::tetrimino::Rotation mRotationID;
		float mBlockSize_;
		sf::Vector2f mOrigin_;
		sf::RectangleShape mBlockShape;
		::model::tetrimino::LocalSpace mPossibleRotations[ (int)::model::tetrimino::Rotation::NONE_MAX ];
		static sf::Vector2<int8_t> Test[ (int)::model::tetrimino::Rotation::NONE_MAX ][ 4 ];
		static sf::Color Colors[ (int)::model::tetrimino::Type::NONE_MAX ];
		static sf::Color BlockOutlineColor;
	};
}