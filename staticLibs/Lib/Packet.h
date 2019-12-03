#pragma once
#include <sstream>
#include "Common.h"

class Packet
{
public:
	Packet( )
		: mHasSomethingToSend( false )
	{ }
	~Packet( ) = default;

	inline void pack( const Tag tag, const std::string& data )
	{
		mData += (tag + std::to_string(data.size()) + TOKEN_SEPARATOR_2 + data + TOKEN_SEPARATOR);
		mHasSomethingToSend = true;
	}
	inline bool hasData( ) const
	{
		return mHasSomethingToSend;
	}
	inline std::string& data( )
	{
		return mData;
	}
private:
	bool mHasSomethingToSend;
	std::string mData;
};