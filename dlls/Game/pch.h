#pragma once

#include <winsdkver.h>
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#include <sdkddkver.h>

#include <SFML/Graphics.hpp> // WIN32_LEAN_AND_MEAN을 정의한 다음에 포함하면 컴파일 에러 발생.
#include <Lib/Socket.h>
#define WIN32_LEAN_AND_MEAN // NOTE: Defined after Socket.h to include UUID.

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
#include <SFML/System.hpp>
#ifdef _DEBUG
#pragma comment( lib, "sfml-window-d" )
#pragma comment( lib, "sfml-graphics-d" )
#pragma comment( lib, "sfml-system-d" )
#else
#pragma comment( lib, "sfml-window" )
#pragma comment( lib, "sfml-graphics" )
#pragma comment( lib, "sfml-system" )
#endif
#include <lua.hpp>
#pragma comment( lib, "lua53" )
#include <Lib/Hash.h>
#include <Lib/EndPoint.h>
#include <Lib/Packet.h>
#include <Lib/math/Vector.h>
#pragma comment( lib, "Lib" )
#include <GameLib/CommandList.h>
#include <GameLib/Common.h>
#include <GameLib/IServiceLocator.h>
#include <GameLib/VaultKeyList.h>

using Clock = std::chrono::high_resolution_clock;

// !IMPORTANT: On delivery, make sure that this macro be undefined for the security.
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
	gService()->console().printFailure( FailureLevel::FATAL, \
										msg + __FILE__ + ":" + std::to_string( __LINE__ ) + ")" ); \
}
#define ASSERT_TRUE( x ) \
if ( true != ( x ) ) \
{ \
	std::string msg( "ASSERT_TRUE disproved. (" ); \
	gService()->console().printFailure( FailureLevel::FATAL, \
										msg + __FILE__ + ":" + std::to_string( __LINE__ ) + ")" ); \
}
#define ASSERT_NOT_NULL( x ) \
if ( nullptr == (x) || NULL == (x) ) \
{ \
	std::string msg( "ASSERT_NOT_NULL disproved. (" ); \
	gService()->console().printFailure( FailureLevel::FATAL, \
										msg + __FILE__ + ":" + std::to_string( __LINE__ ) + ")" ); \
}
#endif