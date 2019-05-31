#include "Sequence.h"
#include "Intro.h"
#include "MainMenu.h"
#include "inPlay/InPlay.h"

bool sequence::Sequence::IsInstanciated = false;

sequence::Sequence::Sequence( sf::RenderWindow& window )
	: mWindow( window ), mNextMainSequence( nullptr ), mCurrentSequence( nullptr )
{
	ASSERT_FALSE( IsInstanciated );
	auto& varT = ::global::VariableTable( );
	// NOTE: '::sequence::Seq' is enum class, not enum, thus expilcit casting is necessary.
	// For more details, try compiling without explicit casting.
	varT.emplace( 1956789523, static_cast< uint32_t >( ::sequence::Seq::INTRO ) ); // nextMainSeq
	// NOTE: 'std::unordered_map::find( )' is safer than 'std::unordered_map::operator[]'
	// as the latter inserts new one if the value to which the key is mapping doesn't exist.
	// That means, [] can't prevent the disaster that two hash values differ from each other.
	// ( e.g. varT.find( 1956789523 ) returns end iterator, but varT[ 1956789523 ] inserts new one. )
	mNextMainSequence = reinterpret_cast< ::sequence::Seq* >( &varT.find( 1956789523 )->second ); // nextMainSeq
	IsInstanciated = true;
}

void sequence::Sequence::update( )
{
	switch ( *mNextMainSequence )
	{
		case ::sequence::Seq::INTRO:
			moveTo< ::sequence::Intro >( );
			break;
		case ::sequence::Seq::MAIN_MENU:
			moveTo< ::sequence::MainMenu >( );
			break;
		case ::sequence::Seq::SINGLE_PLAY://궁금: 따로 만들까?
			[[ fallthrough ]];
		case ::sequence::Seq::MULTI_PLAY:
			moveTo< ::sequence::inPlay::InPlay >( );
			break;
		case ::sequence::Seq::NONE:
			break;
		default:
			__assume( 0 );
			break;
	}
	mCurrentSequence->update( );
}

void sequence::Sequence::draw( )
{
	mCurrentSequence->draw( );
}
