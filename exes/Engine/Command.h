#pragma once
#pragma hdrstop
#include <unordered_map>
#include <string_view>

class Command
{
private:
	friend class ConsoleLocal; //궁금: 이렇게만 해도 되나?  include 해야하나?  IConsole 인터페이스로도 되나?

	Command( );
	~Command( )
	{
		IsInitialized = false;
	}

	static void ProcessCommand( const std::string& command );

	static bool IsInitialized;
	static std::unordered_map< std::string, void(*)( const std::string_view& ) > _Dictionary;
};