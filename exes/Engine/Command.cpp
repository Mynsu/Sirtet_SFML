#include "Command.h"
#include "ServiceLocator.h"
#include "Game/sequence/ISequence.h"

bool Command::IsInitialized = false;
std::unordered_map< std::string, void( *)( const std::string_view& ) > Command::_Dictionary;

void Refresh( const std::string_view& )
{
	auto& varT = ServiceLocator::VariableTable( );
	constexpr HashedKey key0 = ::util::hash::Digest( "nextMainSeq" );
	auto it0 = varT.find( key0 );
	if ( varT.end( ) == it0 )
	{
		::ServiceLocator::Console( )->printError( ErrorLevel::WARNING, "The key not found" );
#ifdef _DEBUG
		__debugbreak( );
#endif
		return;
	}

	if ( ::sequence::Seq::SINGLE_PLAY == static_cast< ::sequence::Seq >( it0->second ) ||
		 ::sequence::Seq::MULTI_PLAY == static_cast< ::sequence::Seq >( it0->second ) )
	{
		constexpr HashedKey key1 = ::util::hash::Digest( "nextInPlaySeq" );
		auto it1 = varT.find( key1 );
		if ( varT.end( ) == it1 )
		{
			::ServiceLocator::Console( )->printError( ErrorLevel::WARNING, "The key not found" );
#ifdef _DEBUG
			__debugbreak( );
#endif
		}

	}
	else
	{

	}
}

Command::Command( )
{
	ASSERT_FALSE( IsInitialized );

	_Dictionary.emplace( "refresh", &Refresh );

	IsInitialized = true;
}

void Command::ProcessCommand( const std::string& command )
{
	// command를 토크나이저로 파싱?해서 ..(렉스는 뭐더라?)
	// 오퍼랜드? 혹은 아규먼트 빼내자.

	// 사전.find( command ) 없으면 끝.
	// 있으면,
	// lua 안 쓰면 값비싼 std::any써야하니까 차라리 lua가 낫겠다.
	 
	// refresh 할 때 사전도 꼭 갱신하고.

	// Parsing command
	// e.g.
	// When command == "refresh", cmd0 == "refresh" but argv == ""
	// When command == "set nextMainSeq 1", cmd0 == "set" and arvg == "nextMainSeq 1"
	std::string_view cmd0;
	std::string_view argv;
	const size_t idxOf1stTokenizer = command.find_first_of( ' ' );
	if ( std::string::npos != idxOf1stTokenizer )
	{
		cmd0 = command;
		cmd0 = cmd0.substr( 0, idxOf1stTokenizer );
		argv = command;
		argv = argv.substr( idxOf1stTokenizer + 1 );
	}
	else
	{
		cmd0 = command;
		argv = std::string( "" );
	}

	if ( const auto& it = _Dictionary.find( cmd0.data( ) ); _Dictionary.cend( ) != it )
	{
		it->second( argv );
	}
	else
	{
		ServiceLocator::Console( )->printError( ErrorLevel::WARNING, "Command '" + command + "' is undefined." );
	}
}
