#pragma once

#include <list>
#include <queue>
#include <array>
#include <bitset>
#include <thread>
#include <condition_variable>
#include <memory>
#include <string>
#include <string_view>
#include <functional>
#include <random>
#include <optional>
#include <intrin.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#ifdef _DEBUG
#pragma comment( lib, "sfml-window-d" )
#pragma comment( lib, "sfml-system-d" )
#pragma comment( lib, "sfml-graphics-d" )
#else
#pragma comment( lib, "sfml-window" )
#pragma comment( lib, "sfml-system" )
#pragma comment( lib, "sfml-graphics" )
#endif
#include <lua.hpp>
#pragma comment( lib, "lua53" )
#include <Lib/Hash.h>
#include <Lib/EndPoint.h>
#include <Lib/Socket.h>
#include <Lib/Packet.h>
#include <Lib/math/Vector.h>
#pragma comment( lib, "Lib-d" )
#include <IServiceLocator.h>
#include <IGame.h>

using Clock = std::chrono::high_resolution_clock;

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
	(*gService).console()->printFailure( FailureLevel::FATAL, \
											msg + __FILE__ + ":" + std::to_string( __LINE__ ) + ")" ); \
}
#define ASSERT_TRUE( x ) \
if ( true != ( x ) ) \
{ \
	std::string msg( "ASSERT_TRUE disproved. (" ); \
	(*gService).console()->printFailure( FailureLevel::FATAL, \
											msg + __FILE__ + ":" + std::to_string( __LINE__ ) + ")" ); \
}
#define ASSERT_NOT_NULL( x ) \
if ( nullptr == (x) || NULL == (x) ) \
{ \
	std::string msg( "ASSERT_NOT_NULL disproved. (" ); \
	(*gService).console()->printFailure( FailureLevel::FATAL, \
											msg + __FILE__ + ":" + std::to_string( __LINE__ ) + ")" ); \
}
#endif