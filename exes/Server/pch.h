#pragma once

#include <Lib/Socket.h>
#include <SFML/Graphics/Color.hpp>
#define WIN32_LEAN_AND_MEAN // NOTE: Defined after Socket.h to include UUID.

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
#include <stdint.h>
#include <intrin.h>
#ifdef _DEBUG
#pragma comment( lib, "sfml-graphics-d" )
#else
#pragma comment( lib, "sfml-graphics" )
#endif
#include <lua.hpp>
#pragma comment( lib, "lua53" )
#include <Lib/IOCP.h>
#include <Lib/EndPoint.h>
#include <Lib/Hash.h>
#include <Lib/Packet.h>
#pragma comment( lib, "Lib" )
#include <GameLib/Common.h>

#define _DEV
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