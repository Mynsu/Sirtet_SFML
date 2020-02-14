#pragma once
#include <string>
#include <intrin.h>

// NOTE: Not to include "Common.h".
using Tag = char[];

class Packet
{
public:
	Packet( )
		: mHasSomethingToSend( false )
	{ }
	~Packet( ) = default;

	void pack( const Tag tag, std::string& data, const bool includingTotalSize = true )
	{
		mData += tag;
		if ( true == includingTotalSize )
		{
			const uint16_t size = ::htons((uint16_t)data.size());
			mData.append( (char*)&size, sizeof(size) );
		}
		mData += data;
		mHasSomethingToSend = true;
	}
	template <typename T, std::enable_if_t<std::is_integral_v<T>>* = nullptr>
	void pack( const Tag tag, T data )
	{
		static_assert( sizeof(T) <= sizeof(uint64_t) );
		mData += tag;
		switch ( sizeof(T) )
		{
			case 1:
				break;
			case 2:
			{
				data = (T)::htons((uint16_t)data);
				break;
			}
			case 4:
			{
				data = (T)::htonl((uint32_t)data);
				break;
			}
			case 8:
			{
				data = (T)::htonll((uint64_t)data);
				break;
			}
			default:
#ifndef _DEBUG
				__assume( 0 );
#endif
				break;
		}
		mData.append( (char*)&data, sizeof(T) );
		mHasSomethingToSend = true;
	}
	bool hasData( ) const
	{
		return mHasSomethingToSend;
	}
	std::string& data( )
	{
		return mData;
	}
private:
	bool mHasSomethingToSend;
	std::string mData;
};