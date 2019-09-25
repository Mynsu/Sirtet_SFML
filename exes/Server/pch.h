#pragma once

#include <Lib/Socket.h>
// NOTE: Defined after Socket.h to include UUID
#define WIN32_LEAN_AND_MEAN

#include <signal.h>
#include <iostream>
#include <Windows.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <stdint.h>
#include <thread>
#include <intrin.h>
#include <lua.hpp>
#include <Lib/IOCP.h>
#include <Lib/EndPoint.h>
#include <Lib/Hash.h>