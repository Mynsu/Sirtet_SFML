#include "pch.h"
#include "Tetrimino.h"

sf::Vector2<int8_t> model::Tetrimino::Test[ static_cast<int>(Rotation::NULL_MAX) ][ 4 ] =
	{ 
		{ {1,0}, {1,-1}, {0,2}, {1,2} },
		{ {-1,0}, {-1,1}, {0,-2}, {-1,-2} },
		{ {-1,0}, {-1,-1}, {0,2}, {-1,2} },
		{ {1,0}, {1,1}, {0,-2}, {1,-2} },
	};

::model::Tetrimino model::Tetrimino::Spawn( )
{
	std::random_device rD;
	std::minstd_rand rE( rD( ) ); //±Ã±Ý: ·¹ÆÛ·±½º ¹®¼­ º¸ÀÚ.
	std::uniform_int_distribution shapeDist( static_cast<int>(::model::tetrimino::Type::I),
											 static_cast<int>(::model::tetrimino::Type::NONE_MAX)-1 ); //±Ã±Ý: ¾êµµ.
	::model::tetrimino::Type type = static_cast<::model::tetrimino::Type>(shapeDist(rE));
	Tetrimino retVal;
	retVal.mType = type;
	switch ( type )
	{
		case ::model::tetrimino::Type::I://TODO
			retVal.mPossibleRotations[ 0 ] = 0b0100'0100'0100'0100;
			retVal.mPossibleRotations[ 1 ] = 0b0000'0000'1111'0000;
			retVal.mPossibleRotations[ 2 ] = 0b0010'0010'0010'0010;
			retVal.mPossibleRotations[ 3 ] = 0b0000'1111'0000'0000;
			retVal.mRotationID = static_cast< Rotation >( 1 );
			retVal.setColor( sf::Color::Cyan );//TODO
			break;
		case ::model::tetrimino::Type::J:
			retVal.mPossibleRotations[ 0 ] = 0b1000'1110'0000'0000;
			retVal.mPossibleRotations[ 1 ] = 0b0100'0100'1100'0000;
			retVal.mPossibleRotations[ 2 ] = 0b0000'1110'0010'0000;
			retVal.mPossibleRotations[ 3 ] = 0b0110'0100'0100'0000;
			retVal.mRotationID = static_cast < Rotation >( 0 );
			retVal.setColor( sf::Color::Blue );//TODO
			break;
		case ::model::tetrimino::Type::L:
			retVal.mPossibleRotations[ 0 ] = 0b0010'1110'0000'0000;
			retVal.mPossibleRotations[ 1 ] = 0b1100'0100'0100'0000;
			retVal.mPossibleRotations[ 2 ] = 0b0000'1110'1000'0000;
			retVal.mPossibleRotations[ 3 ] = 0b0100'0100'0110'0000;
			retVal.mRotationID = static_cast < Rotation >( 0 );
			// Orange
			retVal.setColor( sf::Color( 0xff7f00ff ) );//TODO
			break;
		case ::model::tetrimino::Type::N:
			retVal.mPossibleRotations[ 0 ] = 0b1100'0110'0000'0000;
			retVal.mPossibleRotations[ 1 ] = 0b0100'1100'1000'0000;
			retVal.mPossibleRotations[ 2 ] = 0b0000'1100'0110'0000;
			retVal.mPossibleRotations[ 3 ] = 0b0010'0110'0100'0000;
			retVal.mRotationID = static_cast< Rotation >( 0 );
			retVal.setColor( sf::Color::Red );//TODO
			break;
		case ::model::tetrimino::Type::S:
			retVal.mPossibleRotations[ 0 ] = 0b0110'1100'0000'0000;
			retVal.mPossibleRotations[ 1 ] = 0b0100'0110'0010'0000;
			retVal.mPossibleRotations[ 2 ] = 0b0000'0110'1100'0000;
			retVal.mPossibleRotations[ 3 ] = 0b1000'1100'0100'0000;
			retVal.mRotationID = static_cast< Rotation >( 0 );
			retVal.setColor( sf::Color::Green );//TODO
			break;
		case ::model::tetrimino::Type::T:
			retVal.mPossibleRotations[ 0 ] = 0b0100'1110'0000'0000;
			retVal.mPossibleRotations[ 1 ] = 0b0100'1100'0100'0000;
			retVal.mPossibleRotations[ 2 ] = 0b0000'1110'0100'0000;
			retVal.mPossibleRotations[ 3 ] = 0b0100'0110'0100'0000;
			retVal.mRotationID = static_cast < Rotation >( 0 );
			// Purple - Old Citadel
			retVal.setColor( sf::Color( 0x562f72ff ) );//TODO
			break;
		case ::model::tetrimino::Type::O:
			retVal.mPossibleRotations[ 0 ] = 0b0000'0110'0110'0000;
			retVal.mPossibleRotations[ 1 ] = retVal.mPossibleRotations[ 0 ];//TODO
			retVal.mPossibleRotations[ 2 ] = retVal.mPossibleRotations[ 0 ];//TODO
			retVal.mPossibleRotations[ 3 ] = retVal.mPossibleRotations[ 0 ];//TODO
			retVal.mRotationID = static_cast< Rotation >( 0 );
			retVal.setColor( sf::Color::Yellow );//TODO
			break;
		default:
#ifdef _DEBUG
			__debugbreak( );
#else
			__assume( 0 );
#endif
	}
	retVal.mPosition.x = ::model::stage::GRID_WIDTH/2u - 1u;
	retVal.mPosition.y = 0u;

	return retVal;
}

void model::Tetrimino::tryRotate( const std::array<std::array<::model::Cell,::model::stage::GRID_WIDTH>, ::model::stage::GRID_HEIGHT>& grid )
{
	// Copying prototype
	Tetrimino afterRot( *this );
	//
	// Prototype's rotation
	//
	uint8_t rotID = static_cast< uint8_t >( afterRot.mRotationID );
	++rotID;
	afterRot.mRotationID = ( Rotation::NULL_MAX == static_cast< Rotation >( rotID ) )?
		Rotation::A: static_cast< Rotation >( rotID );
	// When no collision happens,
	if ( false == afterRot.hasCollidedWith( grid ) )
	{
		mRotationID = afterRot.mRotationID;
		return;
	}
	// When a collision is detected,
	// NOTE: 'else' is omitted.
	///else
	//
	// Moving the prototype around here
	//
	for ( uint8_t i = 0u; i != 4u; ++i )
	{
		afterRot.mPosition += Test[ static_cast<int>(afterRot.mRotationID) ][ i ];
		// When the rotation is possible after moving,
		if ( false == afterRot.hasCollidedWith( grid ) )
		{
			mRotationID = afterRot.mRotationID;
			mPosition = afterRot.mPosition;
			return;
		}
		// When the rotation is impossible,
		// NOTE: 'else' is omitted.
		///else
		afterRot.mPosition -= Test[ static_cast<int>(afterRot.mRotationID) ][ i ];
	}
}

bool model::Tetrimino::hasCollidedWith( const std::array<std::array<Cell,::model::stage::GRID_WIDTH>, ::model::stage::GRID_HEIGHT>& grid ) const
{
	bool retVal = false;
	const uint8_t area = ::model::tetrimino::BLOCKS_A_TETRIMINO*::model::tetrimino::BLOCKS_A_TETRIMINO;
	for ( int8_t i = area-1; i != -1; --i )
	{
		if ( mPossibleRotations[static_cast<int>(mRotationID)] & (0x1u<<(area-i-1u)) )
		{
			// Coordinate transformation
			const int8_t x = mPosition.x + i%model::tetrimino::BLOCKS_A_TETRIMINO;
			const int8_t y = mPosition.y + i/model::tetrimino::BLOCKS_A_TETRIMINO;
			if ( ::model::stage::GRID_HEIGHT == y )
			{
				retVal = true;
				break;
			}
			else if ( x < 0 || ::model::stage::GRID_WIDTH <= x )
			{
				retVal = true;
				break;
			}
			else if ( true == grid[ y ][ x ].blocked )
			{
				retVal = true;
				break;
			}
		}
	}
	return retVal;
}
