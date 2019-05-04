#pragma once
#pragma hdrstop
#include "../Common.h"

namespace sequence
{
	// NOTE: GAME_API, __declspec( dllimport/dllexport ) is unnecessary.
	enum class Seq
	{
		OPENING = 0,
		MAIN_MENU,
		NONE,
	};

	class GAME_API ISequence : public sf::NonCopyable
	{
	public:
		ISequence( ) = default;
		virtual ~ISequence( ) = default;

		virtual void update( ) = 0;
		virtual void draw( ) = 0;
	};
}
