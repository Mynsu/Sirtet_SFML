#pragma once
#pragma hdrstop
#include <Lib/precompiled.h>

class Command
{
	friend class ConsoleLocal;
public:
	using Func = std::function< void( const std::string_view& args ) >;
private:
	Command( ) = default;
	~Command( ) = default;

	void addCommand( const HashedKey command, const Func& functional )
	{
		mProtocols.emplace( command, functional );
	}
	void processCommand( const std::string& commandLine );

	std::unordered_map< HashedKey, Func > mProtocols;
};