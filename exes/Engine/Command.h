#pragma once
#pragma hdrstop
#include <unordered_map>
#include <string_view>

class Command
{
private:
	friend class ConsoleLocal; //�ñ�: �̷��Ը� �ص� �ǳ�?  include �ؾ��ϳ�?  IConsole �������̽��ε� �ǳ�?

	Command( );
	~Command( )
	{
		IsInitialized = false;
	}

	static void ProcessCommand( const std::string& command );

	static bool IsInitialized;
	static std::unordered_map< std::string, void(*)( const std::string_view& ) > _Dictionary;
};