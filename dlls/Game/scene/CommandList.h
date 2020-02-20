#pragma once
#include <Lib/Hash.h>

// e.g. "chto 1"
// Change the current scene to another scene.
// Intro 0, Main Menu 1, Singleplay 2, Online Battle 3.
constexpr HashedKey CMD_CHANGE_SCENE = ::util::hash::Digest( "chto" );

// e.g. "refresh"
// Refresh the current scene,
// in other words, reload its script and resources.
constexpr HashedKey CMD_RELOAD = ::util::hash::Digest( "refresh" );

// Used just in Console.cpp
/// constexpr HashedKey CMD_RELOAD_CONSOLE = ::util::hash::Digest( "refreshcon" );

constexpr HashedKey CMD_CREATE_ROOM = ::util::hash::Digest( "croom" );

constexpr HashedKey CMD_START_GAME = ::util::hash::Digest( "start" );

constexpr HashedKey CMD_LEAVE_ROOM = ::util::hash::Digest( "leave" );

// e.g. "join nickname1"
constexpr HashedKey CMD_JOIN_ROOM = ::util::hash::Digest( "join" );