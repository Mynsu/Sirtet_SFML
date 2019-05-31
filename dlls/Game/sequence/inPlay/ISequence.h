#pragma once
#pragma hdrstop
#include "../../Common.h"

namespace sequence::inPlay
{
	enum class Seq
	{
		READY = 0,
		NONE,
	};

	class GAME_API ISequence
	{
	public:
		ISequence( ) = default;
		ISequence( sf::RenderWindow& window, ::sequence::inPlay::Seq* const nextInPlaySequence )
			: iWindow( window ), iNextInPlaySequence( nextInPlaySequence )
		{ }
		ISequence( const ISequence& ) = delete;
		void operator=( const ISequence& ) = delete;
		virtual ~ISequence( ) = default;

		virtual void update( ) = 0;
		virtual void draw( ) = 0;
	protected:
		sf::RenderWindow& iWindow;
		::sequence::inPlay::Seq* const iNextInPlaySequence;
		sf::Texture iTexture;
		sf::Sprite iSprite;
	};
}