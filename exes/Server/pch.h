#pragma once

#include <winsdkver.h>
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#include <sdkddkver.h>

#include <SFML/Graphics/Color.hpp> // WIN32_LEAN_AND_MEAN을 정의한 다음에 포함하면 컴파일 에러 발생.
#include <Lib/Socket.h>
#define WIN32_LEAN_AND_MEAN // NOTE: Defined after Socket.h to include UUID.

#include <stdint.h>
#include <intrin.h>
#include <signal.h>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <array>
#include <bitset>
#include <string>
#include <iostream>
#include <random>
#include <chrono>
#ifdef _DEBUG
#pragma comment( lib, "sfml-graphics-d" )
#else
#pragma comment( lib, "sfml-graphics" )
#endif
#include <Lib/IOCP.h>
#include <Lib/EndPoint.h>
#include <Lib/Hash.h>
#include <Lib/Packet.h>
#pragma comment( lib, "Lib" )
#include <GameLib/Common.h>

///#define _DEV
using Clock = std::chrono::high_resolution_clock;
using ClientIndex = uint16_t;
using Ticket = HashedKey;
using RoomID = uint32_t;
const uint16_t CLIENT_CAPACITY = 100;

#ifdef _DEBUG
#define ASSERT_TRUE( x ) if ( false == (x) ) __debugbreak( );
#else
#define ASSERT_TRUE( x )\
if ( false == (x) )\
{\
	std::string msg( "ASSERT_TRUE disproved. (" );\
	std::cerr << msg + __FILE__ + ':' + std::to_string( __LINE__ ) + ')' << std::endl;\
}
#endif