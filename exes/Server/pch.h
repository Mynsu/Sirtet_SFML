#pragma once

#include <Lib/Socket.h>
// NOTE: Defined after Socket.h to include UUID
#define WIN32_LEAN_AND_MEAN

#include <signal.h>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <array>
#include <bitset>
#include <string>
#include <iostream>
#include <stdint.h>
#include <intrin.h>
#include <random>
#include <chrono>
#include <lua.hpp>
#include <SFML/Graphics.hpp>
#include <Lib/IOCP.h>
#include <Lib/EndPoint.h>
#include <Lib/Hash.h>
#include <Lib/Packet.h>

using Clock = std::chrono::high_resolution_clock;
using ClientIndex = uint32_t;
using Ticket = HashedKey;
using RoomID = uint32_t;

#ifdef _DEBUG
#define ASSERT_TRUE( x ) if ( false == (x) ) __debugbreak( );
#else
#define ASSERT_TRUE( x )\
if ( false == (x) )\
{\
	std::string msg( "ASSERT_TRUE disproved. (" );\
	std::cerr << msg + __FILE__ + ':' + std::to_string( __LINE__ ) + ')'; << std::endl;\
}
#endif