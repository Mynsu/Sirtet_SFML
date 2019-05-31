#pragma once
#pragma hdrstop
#include "../../Common.h"
#include "ISequence.h"

namespace sequence::inPlay
{
	class GAME_API Ready final : public ::sequence::inPlay::ISequence
	{
	public:
		Ready( ) = delete;
		Ready( sf::RenderWindow& window, ::sequence::inPlay::Seq* const nextInPlaySequence );
		~Ready( ) = default;

		void update( ) { };
		void draw( ) { };
	};
}