#include "pch.h"
#include "Tetrimino.h"

sf::Vector2<int8_t> model::Tetrimino::Test[(int)::model::tetrimino::Rotation::NONE_MAX][4] =
	{ 
		{ {1,0}, {1,-1}, {0,2}, {1,2} },
		{ {-1,0}, {-1,1}, {0,-2}, {-1,-2} },
		{ {-1,0}, {-1,-1}, {0,2}, {-1,2} },
		{ {1,0}, {1,1}, {0,-2}, {1,-2} },
	};

::model::Tetrimino model::Tetrimino::Spawn( )
{
	::model::Tetrimino retVal;
	std::random_device rD;
	std::minstd_rand rE( rD() );
	std::uniform_int_distribution shapeDist( (int)::model::tetrimino::Type::I,
											 (int)::model::tetrimino::Type::NONE_MAX-1 );
	const ::model::tetrimino::Type type = (::model::tetrimino::Type)shapeDist(rE);
	retVal.mType = type;
	switch ( type )
	{
		case ::model::tetrimino::Type::I:
			retVal.mPossibleRotations[0] = 0b0100'0100'0100'0100;
			retVal.mPossibleRotations[1] = 0b0000'0000'1111'0000;
			retVal.mPossibleRotations[2] = 0b0010'0010'0010'0010;
			retVal.mPossibleRotations[3] = 0b0000'1111'0000'0000;
			retVal.mRotationID = (::model::tetrimino::Rotation)1;
			// Cyan
			retVal.mColor = sf::Color(0x00e5e5ff);
			break;
		case ::model::tetrimino::Type::J:
			retVal.mPossibleRotations[0] = 0b1000'1110'0000'0000;
			retVal.mPossibleRotations[1] = 0b0100'0100'1100'0000;
			retVal.mPossibleRotations[2] = 0b0000'1110'0010'0000;
			retVal.mPossibleRotations[3] = 0b0110'0100'0100'0000;
			retVal.mRotationID = (::model::tetrimino::Rotation)0;
			// Blue
			retVal.mColor = sf::Color(0x205cffff);
			break;
		case ::model::tetrimino::Type::L:
			retVal.mPossibleRotations[0] = 0b0010'1110'0000'0000;
			retVal.mPossibleRotations[1] = 0b1100'0100'0100'0000;
			retVal.mPossibleRotations[2] = 0b0000'1110'1000'0000;
			retVal.mPossibleRotations[3] = 0b0100'0100'0110'0000;
			retVal.mRotationID = (::model::tetrimino::Rotation)0;
			// Orange
			retVal.mColor = sf::Color(0xffa500ff);
			break;
		case ::model::tetrimino::Type::N:
			retVal.mPossibleRotations[0] = 0b1100'0110'0000'0000;
			retVal.mPossibleRotations[1] = 0b0100'1100'1000'0000;
			retVal.mPossibleRotations[2] = 0b0000'1100'0110'0000;
			retVal.mPossibleRotations[3] = 0b0010'0110'0100'0000;
			retVal.mRotationID = (::model::tetrimino::Rotation)0;
			// Red
			retVal.mColor = sf::Color(0xcc0000ff);
			break;
		case ::model::tetrimino::Type::S:
			retVal.mPossibleRotations[0] = 0b0110'1100'0000'0000;
			retVal.mPossibleRotations[1] = 0b0100'0110'0010'0000;
			retVal.mPossibleRotations[2] = 0b0000'0110'1100'0000;
			retVal.mPossibleRotations[3] = 0b1000'1100'0100'0000;
			retVal.mRotationID = (::model::tetrimino::Rotation)0;
			// Green
			retVal.mColor = sf::Color(0x329932ff);
			break;
		case ::model::tetrimino::Type::T:
			retVal.mPossibleRotations[0] = 0b0100'1110'0000'0000;
			retVal.mPossibleRotations[1] = 0b0100'1100'0100'0000;
			retVal.mPossibleRotations[2] = 0b0000'1110'0100'0000;
			retVal.mPossibleRotations[3] = 0b0100'0110'0100'0000;
			retVal.mRotationID = (::model::tetrimino::Rotation)0;
			// Purple - Old Citadel
			retVal.mColor = sf::Color(0x562f72ff);
			break;
		case ::model::tetrimino::Type::O:
		{
			const uint16_t SQUARE = 0b0000'0110'0110'0000;
			retVal.mPossibleRotations[0] = SQUARE;
			retVal.mPossibleRotations[1] = SQUARE;
			retVal.mPossibleRotations[2] = SQUARE;
			retVal.mPossibleRotations[3] = SQUARE;
			retVal.mRotationID = (::model::tetrimino::Rotation)0;
			// Yellow
			retVal.mColor = sf::Color(0xffff00ff);
			break;
		}
		default:
#ifdef _DEBUG
			__debugbreak( );
#else
			__assume( 0 );
#endif
	}
	retVal.mPosition.x = ::model::stage::GRID_WIDTH/2 - 1;
	retVal.mPosition.y = 0;

	return retVal;
}

void model::Tetrimino::tryRotate( const ::model::stage::Grid& grid )
{
	// Copying prototype
	Tetrimino afterRot( *this );
	//
	// Prototype's rotation
	//
	uint8_t rotID = (uint8_t)afterRot.mRotationID;
	++rotID;
	using Rot = ::model::tetrimino::Rotation;
	afterRot.mRotationID = ( Rot::NONE_MAX == (Rot)rotID )?
		Rot::A: (Rot)rotID;
	if ( false == afterRot.hasCollidedWith( grid ) )
	{
		mRotationID = afterRot.mRotationID;
		return;
	}
	//
	// Moving the prototype around here
	//
	for ( uint8_t i = 0; i != 4; ++i )
	{
		afterRot.mPosition += Test[(int)afterRot.mRotationID][i];
		if ( false == afterRot.hasCollidedWith( grid ) )
		{
			mRotationID = afterRot.mRotationID;
			mPosition = afterRot.mPosition;
		}
		else
		{
			afterRot.mPosition -= Test[(int)afterRot.mRotationID][i];
		}
	}
}

void model::Tetrimino::land( ::model::stage::Grid& grid )
{
	const ::model::LocalSpace blocks = mPossibleRotations[(int)mRotationID ];
	for ( uint8_t i = 0; i != ::model::tetrimino::LOCAL_SPACE_SIZE; ++i )
	{
		if ( blocks & (0x1<<(::model::tetrimino::LOCAL_SPACE_SIZE-i-1)) )
		{
			const uint8_t x = mPosition.x + i%model::tetrimino::BLOCKS_A_TETRIMINO;
			const uint8_t y = mPosition.y + i/model::tetrimino::BLOCKS_A_TETRIMINO;
			ASSERT_TRUE( (x<::model::stage::GRID_WIDTH) && (y<::model::stage::GRID_HEIGHT) );
			grid[y][x].blocked = true;
			grid[y][x].color = mColor;
		}
	}
}

bool model::Tetrimino::hasCollidedWith( const ::model::stage::Grid& grid ) const
{
	bool retVal = false;
	for ( int8_t i = ::model::tetrimino::LOCAL_SPACE_SIZE-1; i != -1; --i )
	{
		if ( mPossibleRotations[(int)mRotationID]
			& (0x1<<(::model::tetrimino::LOCAL_SPACE_SIZE-i-1)) )
		{
			// Coordinate transformation
			const int8_t x = mPosition.x + i%model::tetrimino::BLOCKS_A_TETRIMINO;
			const int8_t y = mPosition.y + i/model::tetrimino::BLOCKS_A_TETRIMINO;
			if ( ::model::stage::GRID_HEIGHT <= y )
			{
				retVal = true;
				break;
			}
			else if ( x < 0 || ::model::stage::GRID_WIDTH <= x )
			{
				retVal = true;
				break;
			}
			else if ( true == grid[y][x].blocked )
			{
				retVal = true;
				break;
			}
		}
	}
	return retVal;
}
