#pragma once
#include <string>
#include <intrin.h>
#include "Common.h"

class Packet
{
public:
	Packet( )
		: mHasSomethingToSend( false )
	{ }
	~Packet( ) = default;

	inline void pack( const Tag tag, std::string& data )
	{
		mData += tag;
		const uint32_t size = ::htonl((uint32_t)data.size());
		mData.append( (char*)&size, sizeof(size) );
		mData += data;
		mHasSomethingToSend = true;
	}
	template < typename T, std::enable_if_t<std::is_integral_v<T>>* = nullptr >
	inline void pack( const Tag tag, T data )
	{
		mData += tag;
		switch ( sizeof(T) )
		{
			case 1:
				mData += (char)data;
				break;
			case 2:
			{
				data = (T)::htons(data);
				break;
			}
			case 4:
			{
				data = (T)::htonl(data);
				break;
			}
			case 8:
			{
				data = (T)::htonll(data);
				break;
			}
			default:
#ifdef _DEBUG
				__debugbreak( );
#else
				__assume( 0 );
#endif
				break;
		}
		mData.append( (char*)&data, sizeof(T) );
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