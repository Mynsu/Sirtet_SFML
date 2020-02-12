#ifndef PCH_H
#define PCH_H

#include <Lib/Socket.h>
// NOTE: Defined after Socket.h to include UUID
#define WIN32_LEAN_AND_MEAN

#include <signal.h>
#include <unordered_set>
#include <list>
#include <bitset>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <intrin.h>
#include <stdint.h>
#include <Lib/IOCP.h>
#include <Lib/EndPoint.h>
#include <Lib/Hash.h>
#include <Lib/Packet.h>

using Clock = std::chrono::high_resolution_clock;

#endif //PCH_H