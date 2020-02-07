#pragma once

#include <list>
#include <unordered_map>
#include <array>
#include <memory>
#include <string>
#include <string_view>
#include <sstream>
#include <iostream>
#include <functional>
#include <thread>
#include <future>
#include <stdint.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <lua.hpp>
#include <Lib/Hash.h>
#include <Lib/Socket.h>
#include <Lib/IServiceLocator.h>
#include <Lib/IGame.h>
#include <Lib/IConsole.h>

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
	gService._console()->printFailure( FailureLevel::FATAL, \
											msg + __FILE__ + ':' + std::to_string( __LINE__ ) + ')' ); \
}
#define ASSERT_TRUE( x ) \
if ( true != ( x ) ) \
{ \
	std::string msg( "ASSERT_TRUE disproved. (" ); \
	gService._console()->printFailure( FailureLevel::FATAL, \
											msg + __FILE__ + ':' + std::to_string( __LINE__ ) + ')' ); \
}
#define ASSERT_NOT_NULL( x ) \
if ( nullptr == (x) || NULL == (x) ) \
{ \
	std::string msg( "ASSERT_NOT_NULL disproved. (" ); \
	gService._console()->printFailure( FailureLevel::FATAL, \
											msg + __FILE__ + ':' + std::to_string( __LINE__ ) + ')' ); \
}
#endif