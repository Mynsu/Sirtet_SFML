////
// Key to vault, global variable container.
// Commonly used in exes/Engine, dlls/Game.
////

#pragma once
#include "../Lib/Hash.h"

// Key to frames per second while the window gains focus.
constexpr HashedKey HK_FORE_FPS = ::util::hash::Digest( "foreFPS" );
// Key to frames per second while the window loses focus.
constexpr HashedKey HK_BACK_FPS = ::util::hash::Digest( "backFPS" );
// Key to the switch.  When it becomes 0, game loop terminates promptly.
constexpr HashedKey HK_IS_RUNNING = ::util::hash::Digest( "isRunning" );
// Key to whether the window gains focus or not.
constexpr HashedKey HK_HAS_GAINED_FOCUS = ::util::hash::Digest( "hasGainedFocus" );