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
		void draw( sf::RenderWindow& window );
		
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
		void move( const ::model::tetrimino::Rotation rotationID, const sf::Vector2<int8_t> destination )
		{
			mRotationID = rotationID;
			mPosition = destination;
		}
		void setOrigin( const sf::Vector2f& origin )
		{
			mOrigin = origin;
		}
		sf::Vector2<int8_t> position( ) const
		{
			return mPosition;
		}
		void setSize( const float blockSize )
		{
			ASSERT_TRUE( 0.f < blockSize );
			mBlockShape.setSize( sf::Vector2f( blockSize, blockSize ) );
			mBlockSize_ = blockSize;
		}
		::model::tetrimino::Type type( ) const
		{
			return mType;
		}
		// Current blocks within their own local space.
		::model::tetrimino::LocalSpace blocks( ) const
		{
			return mPossibleRotations[(int)mRotationID];
		}
		bool isHardDropping( ) const
		{
			return mIsHardDropping;
		}
		void hardDrop( const bool isHardDropping = true )
		{
			mIsHardDropping = isHardDropping;
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
	private:
		bool hasCollidedWith( const ::model::stage::Grid& grid ) const;
		bool mIsHardDropping;
		// X and y on global coordinate.
		// Unit: Grid.
		sf::Vector2<int8_t> mPosition;
		::model::tetrimino::Type mType;
		::model::tetrimino::Rotation mRotationID;
		// Can be figured out from mBlockShape.
		float mBlockSize_;
		// Mapped to the left top position of the stage panel.
		sf::Vector2f mOrigin;
		// NOTE: 테트리미노를 블록 4개로 구성하지만, 덩치가 큰 그래픽 인스턴스까지 4개를 둘 필요는 없습니다.
		sf::RectangleShape mBlockShape;
		// Retains blocks' position in advance.
		::model::tetrimino::LocalSpace mPossibleRotations[(int)::model::tetrimino::Rotation::NONE_MAX];
		// Relative positions to see while detecting collision for each rotation.
		static sf::Vector2<int8_t> Test[(int)::model::tetrimino::Rotation::NONE_MAX][4];
		static sf::Color OutlineColor;
		static sf::Color Colors[(int)::model::tetrimino::Type::NONE_MAX];
	};
}