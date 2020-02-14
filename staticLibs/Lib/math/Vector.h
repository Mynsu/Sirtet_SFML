#pragma once
#include <intrin.h>
#include <cmath>
#include <type_traits>
#include <SFML/System.hpp>

namespace math
{
	template <int N, typename T = float,
		std::enable_if_t<std::is_floating_point_v<T>>* = nullptr>
	class Vector final
	{
	public:
		Vector( )
			: mComponents{ 0.0 }
		{
		}
		template <typename... Ts>
		Vector( const Ts... components )
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

		T magnitude( ) const
		{
			T magnitude = 0.f;
			for ( const T c : mComponents )
			{
				magnitude += c*c;
			}
			magnitude = std::sqrt(magnitude);
			return magnitude;
		}

		Vector normalize( ) const
		{
			Vector retVal( *this );
			const float mag = magnitude();
			for ( T& it : retVal.mComponents )
			{
				it /= mag;
			}
			return retVal;
		}
		T mComponents[N];
	};

	template <int N>
	Vector<N> operator*( const Vector<N> v, const float s )
	{
		Vector<N> retVal;
		for ( uint8_t i = 0; N != i; ++i )
		{
			retVal.mComponents[i] = v.mComponents[i]*s;
		}
		return retVal;
	}

	template <int N>
	void operator*=( Vector<N>& v, const float s )
	{
		for ( uint8_t i = 0; N != i; ++i )
		{
			v.mComponents[i] *= s;
		}
	}

	template <int N>
	Vector<N> operator-( const Vector<N> lh, const Vector<N> rh )
	{
		Vector<N> retVal;
		for ( uint8_t i = 0; N != i; ++i )
		{
			retVal.mComponents[i] = lh.mComponents[i] - rh.mComponents[i];
		}
		return retVal;
	}
}