#ifndef PCH_H
#define PCH_H

#include <Lib/Socket.h>
// NOTE: Defined after Socket.h to include UUID
#define WIN32_LEAN_AND_MEAN

///#include <Windows.h>
#include <signal.h>
#include <list>
#include <bitset>
#include <memory>
#include <string>
#include <string_view>
#include <iostream>
#include <sstream>
#include <thread>
#include <intrin.h>
#include <stdint.h>
#include <Lib/IteratoredQueue.h>
#include <Lib/IOCP.h>
#include <Lib/EndPoint.h>
#include <Lib/Hash.h>
#include <Lib/Packet.h>

#endif //PCH_H