#pragma once
#pragma hdrstop
#include "../Common.h"

namespace sequence
{
	// NOTE: GAME_API, or __declspec( dllimport/dllexport ) is unnecessary.
	enum class Seq
	{
		INTRO = 0,
		MAIN_MENU,
		SINGLE_PLAY,
		MULTI_PLAY,
		NONE,
	};

	class GAME_API ISequence
	{
	public:
		ISequence( ) = default;
		ISequence( const ISequence& ) = delete;
		void operator=( const ISequence& ) = delete;
		virtual ~ISequence( ) = default;

		virtual void update( ) = 0;
		virtual void draw( ) = 0;
	};
}
