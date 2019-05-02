#pragma once
#include "../Common.h"
#include <SFML/System.hpp>

namespace sequence
{
	extern "C" class GAME_API ISequence : public sf::NonCopyable
	{
	public:
		ISequence( ) = default;
		virtual ~ISequence( ) = default;

		virtual void update( ) = 0;
		virtual void draw( ) = 0;
	};
}
