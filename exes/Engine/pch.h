#pragma once

#include <array>
#include <memory>
#include <string>
#include <sstream>
#include <iostream>
#include <functional>
#include <stdint.h>
#include <intrin.h>
#include <SFML/Graphics.hpp> // WIN32_LEAN_AND_MEAN을 정의한 다음에 포함하면 컴파일 에러 발생.
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <SFML/System.hpp>
#ifdef _DEBUG
#pragma comment( lib, "sfml-window-d" )
#pragma comment( lib, "sfml-system-d" )
#pragma comment( lib, "sfml-graphics-d" )
#pragma comment( lib, "sfml-audio-d" )
#else
#pragma comment( lib, "sfml-window" )
#pragma comment( lib, "sfml-system" )
#pragma comment( lib, "sfml-graphics" )
#pragma comment( lib, "sfml-audio" )
#endif
#include <lua.hpp>
#pragma comment( lib, "lua53" )
#include <GameLib/VaultKeyList.h>

#ifdef _DEBUG
#define ASSERT_FALSE( x ) if ( false != (x) ) __debugbreak( )
#define ASSERT_TRUE( x ) if ( true != (x) ) __debugbreak( )
#define ASSERT_NOT_NULL( x ) if ( nullptr == (x) || NULL == (x) ) __debugbreak( )
#else
#include "ServiceLocator.h"
#define ASSERT_FALSE( x ) \
if ( false != ( x ) ) \
{ \
	std::string msg( "ASSERT_FALSE disproved. (" ); \
	gService._console().printFailure( FailureLevel::FATAL, \
											msg + __FILE__ + ':' + std::to_string( __LINE__ ) + ')' ); \
}
#define ASSERT_TRUE( x ) \
if ( true != ( x ) ) \
{ \
	std::string msg( "ASSERT_TRUE disproved. (" ); \
	gService._console().printFailure( FailureLevel::FATAL, \
											msg + __FILE__ + ':' + std::to_string( __LINE__ ) + ')' ); \
}
#define ASSERT_NOT_NULL( x ) \
if ( nullptr == (x) || NULL == (x) ) \
{ \
	std::string msg( "ASSERT_NOT_NULL disproved. (" ); \
	gService._console().printFailure( FailureLevel::FATAL, \
											msg + __FILE__ + ':' + std::to_string( __LINE__ ) + ')' ); \
}
#endif