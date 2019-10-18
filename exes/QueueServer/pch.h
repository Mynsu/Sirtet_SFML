#ifndef PCH_H
#define PCH_H

// TODO: 여기에 미리 컴파일하려는 헤더 추가

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

#endif //PCH_H
