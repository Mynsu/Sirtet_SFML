#include "Command.h"
#include "ServiceLocator.h"

//namespace
//{
//	void Refresh( const std::string_view&, const char )
//	{
//		auto& varT = ServiceLocator::Vault( );
//		constexpr HashedKey key0 = ::util::hash::Digest( "nextMainSeq" );
//		auto it0 = varT.find( key0 );
//		if ( varT.end( ) == it0 )
//		{
//			::ServiceLocator::Console( )->printFailure( FailureLevel::WARNING, "The key not found" );
//#ifdef _DEBUG
//			__debugbreak( );
//#endif
//			return;
//		}
//
//		if ( ::sequence::Seq::SINGLE_PLAY == static_cast< ::sequence::Seq >( it0->second ) ||
//			 ::sequence::Seq::MULTI_PLAY == static_cast< ::sequence::Seq >( it0->second ) )
//		{
//			constexpr HashedKey key1 = ::util::hash::Digest( "nextInPlaySeq" );
//			auto it1 = varT.find( key1 );
//			if ( varT.end( ) == it1 )
//			{
//				::ServiceLocator::Console( )->printFailure( FailureLevel::WARNING, "The key not found" );
//#ifdef _DEBUG
//				__debugbreak( );
//#endif
//			}
//
//		}
//		else
//		{
//
//		}
//	}
//
//	void ChangeSequenceTo( const std::string_view& argv, const char tokenizer )
//	{
//		// Exception: When argv includes a tokenizer, ' ' typically,
//		// or string with numbers like 'hello1234',
//		// ( NOT numbers followed by string like '1234hello' )
//		if ( std::string_view::npos != argv.find( tokenizer ) ||
//			 0 == std::atoi( argv.data( ) ) )
//		{
//			return;
//		}
//
//		//TODO
//		if ( 0 == argv.compare( "10" ) )
//		{
//		}
//	}
//}

void Command::processCommand( const std::string& commandLine )
{
	// lua 안 쓰면 값비싼 std::any써야하니까 차라리 lua가 낫겠다.
	 
	// refresh 할 때 사전도 꼭 갱신하고.

	// Parsing a command line
	const size_t _1stIdxOfTokenizer = commandLine.find_first_of( ' ' );
	// Exception
	if ( 0 == _1stIdxOfTokenizer )
	{
		return;
	}
	
	HashedKey key = NULL;
	std::string_view argv;
	// When the command line has the one token, like 'refresh',
	if ( std::string::npos == _1stIdxOfTokenizer )
	{
		key = ::util::hash::Digest( commandLine.data( ) );
	}
	// When the command line consists of two or more tokens,
	else if ( ' ' != commandLine.at( _1stIdxOfTokenizer + 1 ) )
	{
		// NOTE: substr() of std::string_view costs less than that of std::string.
		const std::string command( commandLine.substr( 0, _1stIdxOfTokenizer ) );
		key = ::util::hash::Digest( command.data( ) );
		argv = commandLine;
		argv = argv.substr( _1stIdxOfTokenizer + 1 );
	}
	// Exception: When the command line looks like 'refresh ', 'refresh  ' and so forth,
	else
	{
		return;
	}

	// When the first token, or command, e.g. 'refresh' or 'moveTo', is coined,
	if ( const auto& it = _mDictionary.find( key ); _mDictionary.cend( ) != it )
	{
		it->second( argv );
	}
	// Exception: Otherwise,
	else
	{
		ServiceLocator::Console( )->printFailure( FailureLevel::WARNING, "Command '" + commandLine + "' isn't coined." );
	}
}
