#pragma once
#pragma hdrstop
#include "../Common.h"

namespace sequence
{
	class GAME_API ISequence : public sf::NonCopyable
	{
	public:
		ISequence( ) = default;
		virtual ~ISequence( ) = default;

		virtual void update( ) = 0;
		virtual void draw( ) = 0;
	};
}
