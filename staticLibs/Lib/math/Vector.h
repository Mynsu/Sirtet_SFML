#pragma once
#include <intrin.h>
#include <cmath>
#include <type_traits>
#include <SFML/System.hpp>

namespace math
{
	template < int N, typename T = float,
		std::enable_if_t<std::is_floating_point_v<T>>* = nullptr >
	class Vector final
	{
	public:
		Vector( )
			: mMagnitude( 0.0 ), mComponents{ 0.0 }
		{
		}
		template < typename... Ts >
		Vector( const Ts... components )
			: mMagnitude( 0.0 )
		{
			uint8_t i = 0;
			T expansion[] = { components... };
			for ( const T c : expansion )
			{
				if ( N <= i )
				{
					__debugbreak( );
				}
				mComponents[i++] = c;
			}
		}
		~Vector( ) = default;

		T magnitude( )
		{
			if ( 0.0 == mMagnitude )
			{
				T magnitudeSquare = 0.f;
				for ( const auto it : mComponents )
				{
					magnitudeSquare += it*it;
				}
				mMagnitude = std::sqrt(magnitudeSquare);
			}
			return mMagnitude;
		}

		Vector normalize( ) const
		{
			Vector retVal( *this );
			for ( auto& it : retVal.mComponents )
			{
				it /= mMagnitude;
			}
			return retVal;
		}
		T mComponents[N];
	private:
		T mMagnitude;
	};

	template < int N >
	Vector<N> operator*( const Vector<N> v, const float s )
	{
		Vector<N> retVal;
		for ( uint8_t i = 0; N != i; ++i )
		{
			retVal.mComponents[i] = v.mComponents[i]*s;
		}
		return retVal;
	}
}