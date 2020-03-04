#pragma once
#include <GameLib/IConsole.h>
#include <Lib/Hash.h>

class Command
{
	friend class Console;
private:
	Command( ) = default;
	~Command( ) = default;

	void addCommand( const HashedKey command, const Func& functional )
	{
		mProtocols.emplace( command, functional );
	}
	void processCommand( const std::string& commandLine );
	void removeCommand( const HashedKey command )
	{
		mProtocols.erase( command );
	}
	void release( )
	{
		mProtocols.clear( );
	}

	std::unordered_map<HashedKey, Func> mProtocols;
};