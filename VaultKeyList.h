////
// Vault Key List
////
// Shared by exes/Engine, dlls/Game.
////

#pragma once
#include <Lib/Hash.h>

// Key to frames per second while the window gains focus.
constexpr HashedKey HK_FORE_FPS = ::util::hash::Digest( "foreFPS", 7 );
// Key to frames per second while the window loses focus.
constexpr HashedKey HK_BACK_FPS = ::util::hash::Digest( "backFPS", 7 );
// Key to the switch.  When it becomes 0, game loop terminates promptly.
constexpr HashedKey HK_IS_RUNNING = ::util::hash::Digest( "isRunning", 9 );

constexpr char _HK_HAS_GAINED_FOCUS[] = "hasGainedFocus";
constexpr HashedKey HK_HAS_GAINED_FOCUS = ::util::hash::Digest( _HK_HAS_GAINED_FOCUS, ::util::hash::Measure(_HK_HAS_GAINED_FOCUS) );