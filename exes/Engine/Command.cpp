#include "Command.h"
#include "ServiceLocator.h"

void Command::processCommand( const std::string& commandLine )
{
	//
	// Parsing a command line
	///
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
	if ( const auto it = mProtocols.find( key ); mProtocols.cend( ) != it )
	{
		it->second( argv );
	}
	// Exception: Otherwise,
	else
	{
		ServiceLocator::Console( )->printFailure( FailureLevel::WARNING, "Command '" + commandLine + "' isn't coined." );
	}
}
