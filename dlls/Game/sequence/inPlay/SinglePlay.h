#pragma once
#pragma hdrstop
#include "../../Common.h"
#include "../ISequence.h"

namespace sequence::inPlay
{
	class GAME_API SinglePlay final : public ISequence
	{
	public:
		SinglePlay( ) = delete;
		SinglePlay( sf::RenderWindow& window,
					::sequence::Seq* nextSequence );
		~SinglePlay( ) = default;

		void update( ) override;
		void draw( ) override;
	private:
		::sequence::Seq* const mNextSequence;
		sf::RenderWindow& mWindow;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}