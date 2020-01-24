#pragma once
#include <Lib/Hash.h>

// e.g. "chto 1"
// Change the current scene to another scene.
// Intro 0, Main Menu 1, Singleplay 2, Online Battle 3.
constexpr char _CMD_CHANGE_SCENE[ ] = "chto";
constexpr HashedKey CMD_CHANGE_SCENE = ::util::hash::Digest( _CMD_CHANGE_SCENE, ::util::hash::Measure(_CMD_CHANGE_SCENE) );

// e.g. "refresh"
// Refresh the current scene,
// in other words, reload its script and resources.
constexpr char _CMD_RELOAD[ ] = "refresh";
constexpr HashedKey CMD_RELOAD = ::util::hash::Digest( _CMD_RELOAD, ::util::hash::Measure(_CMD_RELOAD) );

constexpr char _CMD_CREATE_ROOM[ ] = "croom";
constexpr HashedKey CMD_CREATE_ROOM = ::util::hash::Digest( _CMD_CREATE_ROOM, ::util::hash::Measure(_CMD_CREATE_ROOM) );

constexpr char _CMD_START_GAME[ ] = "start";
constexpr HashedKey CMD_START_GAME = ::util::hash::Digest( _CMD_START_GAME, ::util::hash::Measure(_CMD_START_GAME) );

constexpr char _CMD_LEAVE_ROOM[ ] = "leave";
constexpr HashedKey CMD_LEAVE_ROOM = ::util::hash::Digest( _CMD_LEAVE_ROOM, ::util::hash::Measure(_CMD_LEAVE_ROOM) );

// e.g. "join nickname1"
constexpr char _CMD_JOIN_ROOM[ ] = "join";
constexpr HashedKey CMD_JOIN_ROOM = ::util::hash::Digest( _CMD_JOIN_ROOM, ::util::hash::Measure(_CMD_JOIN_ROOM) );