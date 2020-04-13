#pragma once
#include <intrin.h>
#include <string>
#include <memory>

// NOTE: Not to include "Common.h".
using Tag = char[];

class Packet
{
public:
	Packet( )
		: mHasSomethingToSend( false )
	{ }
	~Packet( ) = default;

	template <int N>
	void pack( const char (&tag)[N], std::string& data, const bool includingTotalSize = true )
	{
		mData += tag;
		if ( true == includingTotalSize )
		{
			const uint16_t size = ::htons((uint16_t)data.size());
			mData.append( (char*)&size, sizeof(size) );
		}
		mData.append( data.data(), data.size() );
		mHasSomethingToSend = true;
	}
	template <typename T, std::enable_if_t<std::is_integral_v<T>>* = nullptr, int N>
	void pack( const char (&tag)[N], T data )
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
#if (_WIN32_WINNT > _WIN32_WINNT_WIN8)
				data = (T)::htonll((uint64_t)data);
#else
				data = (((uint64_t)data >> 56) & 0x00000000000000ffll) |
					   (((uint64_t)data >> 40) & 0x000000000000ff00ll) |
					   (((uint64_t)data >> 24) & 0x0000000000ff0000ll) |
					   (((uint64_t)data >> 8) & 0x00000000ff000000ll) |
					   (((uint64_t)data << 8) & 0x000000ff00000000ll) |
					   (((uint64_t)data << 24) & 0x0000ff0000000000ll) |
					   (((uint64_t)data << 40) & 0x00ff000000000000ll) |
					   (((uint64_t)data << 56) & 0xff00000000000000ll);
#endif
				break;
			}
			default:
#ifdef _DEBUG
				__debugbreak( );
#else
				__assume( 0 );
#endif
		}
		mData.append( (char*)std::addressof(data), sizeof(T) );
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