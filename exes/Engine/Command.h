#pragma once

class Command
{
	friend class Console;
private:
	Command( ) = default;
	~Command( ) = default;

	inline void addCommand( const HashedKey command, const Func& functional )
	{
		mProtocols.emplace( command, functional );
	}
	void processCommand( const std::string& commandLine );
	inline void removeCommand( const HashedKey command )
	{
		mProtocols.erase( command );
	}
	inline void release( )
	{
		mProtocols.clear( );
	}

	std::unordered_map< HashedKey, Func > mProtocols;
};