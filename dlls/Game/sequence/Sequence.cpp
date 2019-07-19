#include "Sequence.h"
#include "Intro.h"
#include "MainMenu.h"
#include "inPlay/InPlay.h"

bool sequence::Sequence::IsInstanciated = false;

sequence::Sequence::Sequence( sf::RenderWindow& window )
	: mWindow( window ), mNextMainSequence( nullptr ), mCurrentSequence( nullptr )
{
	ASSERT_FALSE( IsInstanciated );
	Dword value = static_cast< Dword >( ::sequence::Seq::INTRO );
	const std::string scriptPathNName( "Scripts/_OnlyDuringDev.lua" );
	const std::string varName( "StartMainSequence" );
	const auto table = ::ServiceLocator::LoadFromScript( scriptPathNName, varName );
	if ( const auto& it = table.find( varName ); table.cend( ) != it )
	{
		value = std::get< int >( it->second );
	}
	auto& varT = ::global::VariableTable( );
	constexpr HashedKey key = util::hash::Digest( "nextMainSeq" );
	varT.emplace( key, value );
	// NOTE: For finding the key-value pair a table already has,
	// 'std::unordered_map::find( )' is safer than 'std::unordered_map::operator[].'
	// That's because the latter inserts new one even if there is no such key.
	// That means, the latter can bring on a disaster.
	// ( e.g. varT.find( 1956789523 ) returns end iterator, but varT[ 1956789523 ] inserts new one. )
	if ( const auto& it = varT.find( key ); varT.cend( ) != it )
	{
		mNextMainSequence = reinterpret_cast< ::sequence::Seq* >( &it->second );
	}
#ifdef _DEBUG
	else
	{
		__debugbreak( );
	}
#endif
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
