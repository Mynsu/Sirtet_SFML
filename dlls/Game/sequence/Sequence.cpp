#include "Sequence.h"
#include "Opening.h"
#include "MainMenu.h"
#include "inPlay/InPlay.h"

sequence::Sequence::Sequence( sf::RenderWindow& window )
	: mWindow( window ),
	mNextSequence( reinterpret_cast< ::sequence::Seq* >( &( ::global::VariableTable( ).find( 2746935819 )->second ) ) )
{
}

void sequence::Sequence::update( )
{
	switch ( *mNextSequence )
	{
#ifdef _DEBUG
#define ASSERT_NONE( x ) if ( ::sequence::Seq::NONE != x ) __debugbreak( )
#else
#define ASSERT_NONE( x ) __assume( true )
#endif
		case ::sequence::Seq::OPENING:
			mCurrentSequence.reset( nullptr );
			mCurrentSequence = std::make_unique< ::sequence::Opening >( mWindow, mNextSequence );
			ASSERT_NONE( *mNextSequence );
			break;
		case ::sequence::Seq::MAIN_MENU:
			mCurrentSequence.reset( nullptr );
			mCurrentSequence = std::make_unique < ::sequence::MainMenu >( mWindow, mNextSequence );
			ASSERT_NONE( *mNextSequence );
			break;
		case ::sequence::Seq::SINGLE_PLAY://궁금: 따로 만들까?
		case ::sequence::Seq::MULTI_PLAY:
			mCurrentSequence.reset( nullptr );
			mCurrentSequence = std::make_unique < ::sequence::inPlay::InPlay >( mWindow, mNextSequence );
			ASSERT_NONE( *mNextSequence );
			break;
		case ::sequence::Seq::NONE:
			break;
		default: __debugbreak( ); break;
	}
	mCurrentSequence->update( );
}

void sequence::Sequence::draw( )
{
	mCurrentSequence->draw( );
}
