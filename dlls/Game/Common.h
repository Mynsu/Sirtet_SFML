//TODO: precompile�ұ�?

#pragma once
#pragma hdrstop

#ifdef GAME_EXPORTS
#define GAME_API __declspec( dllexport )
#else
#define GAME_API __declspec( dllimport )
#endif

#include "../exes/Engine/Console.h"
// NOTE: I don't like std::shared_ptr, so chose lovely raw pointer.
// TODO: Ŭ������ ������ &�� �� �� �ְ�, ���嵵 ������� �� �ְ�...
extern "C" GAME_API IConsole* Console_;

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>