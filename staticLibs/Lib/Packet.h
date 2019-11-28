#pragma once
#include <sstream>
#include "Common.h"

class Packet
{
public:
	Packet( ) = default;
	~Packet( ) = default;

	inline void pack( const Tag tag, const std::string& data )
	{
		mData += (tag + std::to_string(data.size()) + TOKEN_SEPARATOR_2 + data + TOKEN_SEPARATOR);
	}
	inline std::string& data( )
	{
		return mData;
	}
private:
	std::string mData;
};