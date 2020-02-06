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
		IteratoredQueue( )
		{ }
		~IteratoredQueue( ) = default;

		void reserve( const uint32_t max )
		{
			mForRandomAccess.reserve( max );
		}
		void emplace_back( T&& value, const size_t index )
		{
			mQueue.emplace_back( std::make_pair(std::forward(value), index) );
			mForRandomAccess.emplace( index );
		}
		T& front( )
		{
			return mQueue.front( );
		}
		void pop_front( )
		{
			mForRandomAccess.erase( mQueue.front( ).second );
			mQueue.pop_front( );
		}
		bool contains( const size_t index ) const
		{
			const size_t cnt = mForRandomAccess.count( index );
#ifdef _DEBUG
			if ( 1 < cnt )
			{
				__debugbreak( );
			}
#endif
			return (1==cnt)? true: false;
		}
		uint32_t size( ) const
		{
			return (uint32_t)mQueue.size();
		}
	private:
		std::list< std::pair<T,size_t> > mQueue;
		std::unordered_set< size_t > mForRandomAccess;
	};

	// Iterator and random access is available.
	// NOTE: SPINAE doesn't work below.
	/// template < typename T, std::enable_if_t<std::is_arithmetic_v<T>||std::is_pointer_v<T>>* = nullptr >
	template < >
	class IteratoredQueue< uint32_t >
	{
	public:
		IteratoredQueue( ) = default;
		~IteratoredQueue( ) = default;

		void reserve( const uint32_t max )
		{
			mForRandomAccess.reserve( max );
		}
		void emplace_back( const uint32_t value )
		{
			mQueue.emplace_back( value );
			mForRandomAccess.emplace( value );
		}
		uint32_t front( )
		{
			return mQueue.front( );
		}
		void pop_front( )
		{
			mForRandomAccess.erase( mQueue.front() );
			mQueue.pop_front( );
		}
		bool contains( const uint32_t index ) const
		{
			const size_t cnt = mForRandomAccess.count( index );
#ifdef _DEBUG
			if ( 1 < cnt )
			{
				__debugbreak( );
			}
#endif
			return (1==cnt)? true: false;
		}
		uint32_t size( ) const
		{
			return (uint32_t)mQueue.size();
		}
	private:
		std::list< uint32_t > mQueue;
		std::unordered_set< uint32_t > mForRandomAccess;
	};
}