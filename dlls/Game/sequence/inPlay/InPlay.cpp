#include "InPlay.h"
#include "Ready.h"
#include "Playing.h"

bool sequence::inPlay::InPlay::IsInstanciated = false;

sequence::inPlay::InPlay::InPlay( sf::RenderWindow& window,
							  ::sequence::Seq* const nextMainSequence )
	: mWindow( window ),
	mNextMainSequence( nextMainSequence )
{
	ASSERT_FALSE( IsInstanciated );
	*mNextMainSequence = ::sequence::Seq::NONE;

	/*auto& varT = ::global::Vault( );
	constexpr HashedKey key = ::util::hash::Digest( "nextInPlaySeq" );
	varT.emplace( key, static_cast< Dword >( ::sequence::inPlay::Seq::READY ) );
	if ( const auto& it = varT.find( key ); varT.cend( ) != it )
	{
		mNextInPlaySequence = reinterpret_cast< ::sequence::inPlay::Seq* >( &it->second );
	}
#ifdef _DEBUG
	else
	{
		__debugbreak( );
	}
#endif*/
	mBackgroundRect.setSize( sf::Vector2f( mWindow.getSize( ) ) );
	IsInstanciated = true;
}

void sequence::inPlay::InPlay::update( )
{
	switch ( *mNextInPlaySequence )
	{
		case ::sequence::inPlay::Seq::READY:
			moveTo< ::sequence::inPlay::Ready >( );
			break;
		case ::sequence::inPlay::Seq::PLAYING:
			moveTo< ::sequence::inPlay::Playing >( );
			break;
		case ::sequence::inPlay::Seq::NONE:
			break;
		default:
			__assume( 0 );
			break;
	}
	mCurrentInPlaySequence->update( );
}

void sequence::inPlay::InPlay::draw( )
{
	mCurrentInPlaySequence->draw( );
}
