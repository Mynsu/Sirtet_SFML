////
//  Commonly used in exes/Engine, dlls/Game.
////

#pragma once
#include "../Lib/Hash.h"

// e.g. "chto 1"
// Changes the current scene to another scene immediately.
// Intro 0, Main Menu 1, Singleplay 2, Online Battle 3.
// Countdown Skipped 21, Game Over 22, All Levels Cleared 23
constexpr HashedKey CMD_CHANGE_SCENE = ::util::hash::Digest( "chto" );
constexpr HashedKey CMD_SET_VOLUME = ::util::hash::Digest( "vol" );

// e.g. "refresh"
// Refreshes the current scene, or reloads its script and resources.
constexpr HashedKey CMD_RELOAD = ::util::hash::Digest( "refresh" );
constexpr HashedKey CMD_RELOAD_CONSOLE = ::util::hash::Digest( "refreshcon" );

constexpr HashedKey CMD_CREATE_ROOM = ::util::hash::Digest( "croom" );
constexpr HashedKey CMD_START_GAME = ::util::hash::Digest( "start" );
constexpr HashedKey CMD_LEAVE_ROOM = ::util::hash::Digest( "leave" );
// e.g. "join nickname1"
constexpr HashedKey CMD_JOIN_ROOM = ::util::hash::Digest( "join" );