#ifndef PCH_H
#define PCH_H

#include <winsdkver.h>
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#include <sdkddkver.h>

#include <Lib/Socket.h>
#define WIN32_LEAN_AND_MEAN // NOTE: Defined after Socket.h to include UUID.

#include <stdint.h>
#include <intrin.h>
#include <signal.h>
#include <unordered_set>
#include <list>
#include <bitset>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <Lib/IOCP.h>
#include <Lib/EndPoint.h>
#include <Lib/Hash.h>
#include <Lib/Packet.h>
#pragma comment( lib, "Lib" )
#include <GameLib/Common.h>

using Clock = std::chrono::high_resolution_clock;

#endif //PCH_H