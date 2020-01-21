#pragma once
#include <Lib/Hash.h>

// e.g. "chscnto 1"
// Change the current scene to the specifically NUMBERed scene.
// As for the version 8191015,
// Intro 0, Main Menu 1, Singleplay 2, Multiplay 3.
// Added in ::scene::SceneManager.cpp.
constexpr char _CMD_CHANGE_SCENE[ ] = "chto";
constexpr HashedKey CMD_CHANGE_SCENE = ::util::hash::Digest( _CMD_CHANGE_SCENE, ::util::hash::Measure(_CMD_CHANGE_SCENE) );

// e.g. "refresh"
// Refresh the current scene,
// in other words, reload its script and resources.
// Added in ::scene::SceneManager.cpp.
constexpr char _CMD_RELOAD[ ] = "refresh";
constexpr HashedKey CMD_RELOAD = ::util::hash::Digest( _CMD_RELOAD, ::util::hash::Measure(_CMD_RELOAD) );

// e.g. "crtRoom"
// Request room creation to the main server.
// Added in ::scene::online::InLobby.cpp.
constexpr char _CMD_CREATE_ROOM[ ] = "croom";
constexpr HashedKey CMD_CREATE_ROOM = ::util::hash::Digest( _CMD_CREATE_ROOM, ::util::hash::Measure(_CMD_CREATE_ROOM) );

constexpr char _CMD_CANCEL_CONNECTION[ ] = "discon";
constexpr HashedKey CMD_CANCEL_CONNECTION = ::util::hash::Digest( _CMD_CANCEL_CONNECTION, ::util::hash::Measure(_CMD_CANCEL_CONNECTION) );

constexpr char _CMD_START_GAME[ ] = "start";
constexpr HashedKey CMD_START_GAME = ::util::hash::Digest( _CMD_START_GAME, ::util::hash::Measure(_CMD_START_GAME) );

constexpr char _CMD_LEAVE_ROOM[ ] = "leave";
constexpr HashedKey CMD_LEAVE_ROOM = ::util::hash::Digest( _CMD_LEAVE_ROOM, ::util::hash::Measure(_CMD_LEAVE_ROOM) );

constexpr char _CMD_JOIN_ROOM[ ] = "join";
constexpr HashedKey CMD_JOIN_ROOM = ::util::hash::Digest( _CMD_JOIN_ROOM, ::util::hash::Measure(_CMD_JOIN_ROOM) );