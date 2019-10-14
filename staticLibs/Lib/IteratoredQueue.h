#pragma once
#include <list>
#include <unordered_set>
#include <type_traits>

namespace container
{
	// Iterator and random access is available.
	// NOTE: SPINAE doesn't work below.
	/// template < typename T, std::enable_if_t<std::is_class_v<T>>* = nullptr >
	template < typename T >
	class IteratoredQueue
	{
	public:
		template < std::enable_if_t<std::is_class_v<T>>* = nullptr >
		inline IteratoredQueue( )
		{ }
		inline ~IteratoredQueue( ) = default;

		inline void reserve( const uint32_t max )
		{
			mFlags.reserve( max );
		}
		inline void emplace_back( T&& value, const size_t index )
		{
			mBody.emplace_back( std::make_pair(std::forward(value), index) );
			mFlags.emplace( index );
		}
		inline T& front( )
		{
			return mBody.front( );
		}
		inline void pop_front( )
		{
			mFlags.erase( mBody.front( ).second );
			mBody.pop_front( );
		}
		// Returns -1 when containing 2 or more, otherwise return 0 or 1.
		inline int8_t contains( const size_t index )
		{
			size_t cnt = mFlags.count( index );
			if ( 1 < cnt )
			{
				return -1;
			}
			else
			{
				return static_cast< int8_t >( cnt );
			}
		}
		inline uint32_t size( ) const
		{
			return static_cast< uint32_t >( mBody.size( ) );
		}
	private:
		std::list< std::pair<T,size_t> > mBody;
		// 4 bytes for x86, 8 bytes for x64.
		std::unordered_set< size_t > mFlags;
	};

	// Iterator and random access is available.
	// NOTE: SPINAE doesn't work below.
	/// template < typename T, std::enable_if_t<std::is_arithmetic_v<T>||std::is_pointer_v<T>>* = nullptr >
	template < >
	class IteratoredQueue< uint32_t >
	{
	public:
		inline IteratoredQueue( ) = default;
		inline ~IteratoredQueue( ) = default;

		inline void reserve( const uint32_t max )
		{
			mFlags.reserve( max );
		}
		inline void emplace_back( const uint32_t value )
		{
			mBody.emplace_back( value );
			mFlags.emplace( value );
		}
		inline uint32_t front( )
		{
			return mBody.front( );
		}
		inline void pop_front( )
		{
			mFlags.erase( mBody.front() );
			mBody.pop_front( );
		}
		// Returns -1 when containing 2 or more, otherwise return 0 or 1.
		inline int8_t contains( const size_t index )
		{
			size_t cnt = mFlags.count( index );
			if ( 1 < cnt )
			{
				return -1;
			}
			else
			{
				return static_cast< int8_t >( cnt );
			}
		}
		inline uint32_t size( ) const
		{
			return static_cast< uint32_t >( mBody.size( ) );
		}
	private:
		std::list< uint32_t > mBody;
		// NOTE: 4 bytes for x86, 8 bytes for x64.
		std::unordered_set< size_t > mFlags;
	};
}