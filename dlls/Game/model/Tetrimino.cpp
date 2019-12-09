#include "../pch.h"
#include "Tetrimino.h"

sf::Vector2<int8_t> model::Tetrimino::Test[ (int)::model::tetrimino::Rotation::NONE_MAX ][ 4 ] =
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
	retVal.setType( type );

	return retVal;
}

void model::Tetrimino::tryRotate( const ::model::stage::Grid& grid )
{
	// Copying prototype
	Tetrimino afterRot( *this );
	//
	// Prototype's rotation
	//
	uint8_t rotID = static_cast< uint8_t >( afterRot.mRotationID );
	++rotID;
	using Rot = ::model::tetrimino::Rotation;
	afterRot.mRotationID = ( Rot::NONE_MAX == (Rot)rotID )?
		Rot::A: (Rot)rotID;
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

void model::Tetrimino::land( ::model::stage::Grid& grid )
{
	const ::model::tetrimino::LocalSpace blocks = mPossibleRotations[(int)mRotationID ];
	for ( uint8_t i = 0u; i != ::model::tetrimino::LOCAL_SPACE_SIZE; ++i )
	{
		if ( blocks & (0x1u<<(::model::tetrimino::LOCAL_SPACE_SIZE-i-1u)) )
		{
			const uint8_t x = mPosition.x + i%model::tetrimino::BLOCKS_A_TETRIMINO;
			const uint8_t y = mPosition.y + i/model::tetrimino::BLOCKS_A_TETRIMINO;
			ASSERT_TRUE( (x<::model::stage::GRID_WIDTH) && (y<::model::stage::GRID_HEIGHT) );
			grid[ y ][ x ].blocked = true;
			grid[ y ][ x ].color = mBlockShape.getFillColor( );
		}
	}
}

bool model::Tetrimino::hasCollidedWith( const ::model::stage::Grid& grid ) const
{
	bool retVal = false;
	for ( int8_t i = ::model::tetrimino::LOCAL_SPACE_SIZE-1; i != -1; --i )
	{
		if ( mPossibleRotations[static_cast<int>(mRotationID)]
			& (0x1u<<(::model::tetrimino::LOCAL_SPACE_SIZE-i-1u)) )
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
			else if ( true == grid[ y ][ x ].blocked )
			{
				retVal = true;
				break;
			}
		}
	}
	return retVal;
}

void model::Tetrimino::setType( const::model::tetrimino::Type type )
{
	mType = type;
	switch ( type )
	{
		case ::model::tetrimino::Type::I://TODO
			mPossibleRotations[ 0 ] = 0b0100'0100'0100'0100;
			mPossibleRotations[ 1 ] = 0b0000'0000'1111'0000;
			mPossibleRotations[ 2 ] = 0b0010'0010'0010'0010;
			mPossibleRotations[ 3 ] = 0b0000'1111'0000'0000;
			mRotationID = (::model::tetrimino::Rotation)1;
			setColor( sf::Color::Cyan );//TODO
			break;
		case ::model::tetrimino::Type::J:
			mPossibleRotations[ 0 ] = 0b1000'1110'0000'0000;
			mPossibleRotations[ 1 ] = 0b0100'0100'1100'0000;
			mPossibleRotations[ 2 ] = 0b0000'1110'0010'0000;
			mPossibleRotations[ 3 ] = 0b0110'0100'0100'0000;
			mRotationID = (::model::tetrimino::Rotation)0;
			setColor( sf::Color::Blue );//TODO
			break;
		case ::model::tetrimino::Type::L:
			mPossibleRotations[ 0 ] = 0b0010'1110'0000'0000;
			mPossibleRotations[ 1 ] = 0b1100'0100'0100'0000;
			mPossibleRotations[ 2 ] = 0b0000'1110'1000'0000;
			mPossibleRotations[ 3 ] = 0b0100'0100'0110'0000;
			mRotationID = (::model::tetrimino::Rotation)0;
			// Orange
			setColor( sf::Color( 0xff7f00ff ) );//TODO
			break;
		case ::model::tetrimino::Type::N:
			mPossibleRotations[ 0 ] = 0b1100'0110'0000'0000;
			mPossibleRotations[ 1 ] = 0b0100'1100'1000'0000;
			mPossibleRotations[ 2 ] = 0b0000'1100'0110'0000;
			mPossibleRotations[ 3 ] = 0b0010'0110'0100'0000;
			mRotationID = (::model::tetrimino::Rotation)0;
			setColor( sf::Color::Red );//TODO
			break;
		case ::model::tetrimino::Type::S:
			mPossibleRotations[ 0 ] = 0b0110'1100'0000'0000;
			mPossibleRotations[ 1 ] = 0b0100'0110'0010'0000;
			mPossibleRotations[ 2 ] = 0b0000'0110'1100'0000;
			mPossibleRotations[ 3 ] = 0b1000'1100'0100'0000;
			mRotationID = (::model::tetrimino::Rotation)0;
			setColor( sf::Color::Green );//TODO
			break;
		case ::model::tetrimino::Type::T:
			mPossibleRotations[ 0 ] = 0b0100'1110'0000'0000;
			mPossibleRotations[ 1 ] = 0b0100'1100'0100'0000;
			mPossibleRotations[ 2 ] = 0b0000'1110'0100'0000;
			mPossibleRotations[ 3 ] = 0b0100'0110'0100'0000;
			mRotationID = (::model::tetrimino::Rotation)0;
			// Purple - Old Citadel
			setColor( sf::Color( 0x562f72ff ) );//TODO
			break;
		case ::model::tetrimino::Type::O:
			mPossibleRotations[ 0 ] = 0b0000'0110'0110'0000;
			mPossibleRotations[ 1 ] = mPossibleRotations[ 0 ];//TODO
			mPossibleRotations[ 2 ] = mPossibleRotations[ 0 ];//TODO
			mPossibleRotations[ 3 ] = mPossibleRotations[ 0 ];//TODO
			mRotationID = (::model::tetrimino::Rotation)0;
			setColor( sf::Color::Yellow );//TODO
			break;
		default:
#ifdef _DEBUG
			__debugbreak( );
#else
			__assume( 0 );
#endif
	}
	mPosition.x = ::model::stage::GRID_WIDTH/2u - 1u;
	mPosition.y = 0u;
}
