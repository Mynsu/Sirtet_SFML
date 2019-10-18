#pragma once

#include <Lib/Socket.h>
// NOTE: Defined after Socket.h to include UUID
#define WIN32_LEAN_AND_MEAN

#include <signal.h>
#include <Windows.h>
#include <thread>
#include <unordered_set>
#include <string>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <intrin.h>
#include <lua.hpp>
#include <Lib/IteratoredQueue.h>
#include <Lib/IOCP.h>
#include <Lib/EndPoint.h>
#include <Lib/Hash.h>