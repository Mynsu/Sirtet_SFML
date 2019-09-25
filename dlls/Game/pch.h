#pragma once

#include <list>
#include <queue>
#include <array>
#include <bitset>
#include <string>
#include <string_view>
#include <memory>
#include <intrin.h>
#include <stdint.h>
#include <functional>
#include <random>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <lua.hpp>
#include <Lib/Hash.h>
#include <Lib/Socket.h>

// !IMPORTANT: On delivery, make sure that this macro is canceled, or undefined for the security.
#define _DEV

#ifdef _DEBUG
#define ASSERT_FALSE( x ) if ( false != (x) ) __debugbreak( )
#define ASSERT_TRUE( x ) if ( true != (x) ) __debugbreak( )
#define ASSERT_NOT_NULL( x ) if ( nullptr == (x) || NULL == (x) ) __debugbreak( )
#else
#include "ServiceLocatorMirror.h"
#define ASSERT_FALSE( x ) \
if ( false != ( x ) ) \
{ \
	std::string msg( "ASSERT_FALSE disproved. (" ); \
	(*glpService).console()->printFailure( FailureLevel::FATAL, \
											msg + __FILE__ + ":" + std::to_string( __LINE__ ) + ")" ); \
}
#define ASSERT_TRUE( x ) \
if ( true != ( x ) ) \
{ \
	std::string msg( "ASSERT_TRUE disproved. (" ); \
	(*glpService).console()->printFailure( FailureLevel::FATAL, \
											msg + __FILE__ + ":" + std::to_string( __LINE__ ) + ")" ); \
}
#define ASSERT_NOT_NULL( x ) \
if ( nullptr == (x) || NULL == (x) ) \
{ \
	std::string msg( "ASSERT_NOT_NULL disproved. (" ); \
	(*glpService).console()->printFailure( FailureLevel::FATAL, \
											msg + __FILE__ + ":" + std::to_string( __LINE__ ) + ")" ); \
}
#endif