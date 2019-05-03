//TODO: precompile할까?

#pragma once
#pragma hdrstop

#ifdef GAME_EXPORTS
#define GAME_API __declspec( dllexport )
#else
#define GAME_API __declspec( dllimport )
#endif

#include "../exes/Engine/Console.h"
// NOTE: I don't like std::shared_ptr, so chose lovely raw pointer.
// TODO: 클래스에 담으면 &도 쓸 수 있고, 사운드도 묶어놓을 수 있고...
extern "C" GAME_API IConsole* Console_;

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>