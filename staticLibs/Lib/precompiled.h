#pragma once

///#include "targetver.h"
#define WIN32_LEAN_AND_MEAN

#include <memory>
#include <unordered_map>
#include <queue>
#include <string>
#include <string_view>
#include <functional>
#include <intrin.h> // __debugbreak()

#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"

#include "Hash.h"
#include "Endian.h"

using Dword = int32_t;

#ifdef _DEBUG
#define ASSERT_FALSE( x ) if ( false != (x) ) __debugbreak( )
#define ASSERT_TRUE( x ) if ( true != (x) ) __debugbreak( )
#define ASSERT_NOT_NULL( x ) if ( nullptr == (x) || NULL == (x) ) __debugbreak( )
#else
#ifdef GAME_EXPORTS
#define ASSERT_FALSE( x ) \
if ( false != ( x ) ) \
{ \
std::string msg( "ASSERT_FALSE disproved. (" ); \
ServiceLocatorMirror::Console( )->printFailure( FailureLevel::FATAL, \
												msg + __FILE__ + ":" + std::to_string( __LINE__ ) + ")" ); \
}
#define ASSERT_TRUE( x ) \
if ( true != ( x ) ) \
{ \
std::string msg( "ASSERT_TRUE disproved. (" ); \
ServiceLocatorMirror::Console( )->printFailure( FailureLevel::FATAL, \
												msg + __FILE__ + ":" + std::to_string( __LINE__ ) + ")" ); \
}
#define ASSERT_NOT_NULL( x ) \
if ( nullptr == (x) || NULL == (x) ) \
{ \
std::string msg( "ASSERT_NOT_NULL disproved. (" ); \
ServiceLocatorMirror::Console( )->printFailure( FailureLevel::FATAL, \
												msg + __FILE__ + ":" + std::to_string( __LINE__ ) + ")" ); \
}
#else
#define ASSERT_FALSE( x ) \
if ( false != ( x ) ) \
{ \
std::string msg( "ASSERT_FALSE disproved. (" ); \
ServiceLocator::Console( )->printFailure( FailureLevel::FATAL, \
										  msg + __FILE__ + ":" + std::to_string( __LINE__ ) + ")" ); \
}
#define ASSERT_TRUE( x ) \
if ( true != ( x ) ) \
{ \
std::string msg( "ASSERT_TRUE disproved. (" ); \
ServiceLocator::Console( )->printFailure( FailureLevel::FATAL, \
										msg + __FILE__ + ":" + std::to_string( __LINE__ ) + ")" ); \
}
#define ASSERT_NOT_NULL( x ) \
if ( nullptr == (x) || NULL == (x) ) \
{ \
std::string msg( "ASSERT_NOT_NULL disproved. (" ); \
ServiceLocator::Console( )->printFailure( FailureLevel::FATAL, \
												msg + __FILE__ + ":" + std::to_string( __LINE__ ) + ")" ); \
}
#endif
#endif

// !IMPORTANT: On delivery, make sure that this macro is canceled, or undefined for the security.
#define _DEV