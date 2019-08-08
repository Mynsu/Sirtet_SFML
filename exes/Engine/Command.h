#pragma once
#pragma hdrstop
#include <unordered_map>
#include <Lib/Hash.h>
#include <functional>
#include <string_view>

class Command
{
	friend class ConsoleLocal; //�ñ�: IConsole �������̽��ε� �ǳ�?
public:
	using Func = std::function< void( const std::string_view& args ) >;
private:
	Command( ) = default;
	~Command( ) = default;

	void addCommand( const HashedKey command, const Func& functional )
	{
		_mDictionary.emplace( command, functional );
	}
	void processCommand( const std::string& commandLine );

	std::unordered_map< HashedKey, Func > _mDictionary;//�ñ�: HashedKey�� ��� ������?
};