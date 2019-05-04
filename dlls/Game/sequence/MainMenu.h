#pragma once
#pragma hdrstop
#include "../Common.h"
#include "ISequence.h"

namespace sequence
{
	class GAME_API MainMenu final : public ISequence
	{
	public:
		MainMenu( ) = delete;
		MainMenu( sf::RenderWindow& window, ::sequence::Seq* const nextSequence );
		~MainMenu( ) = default;

		void update( );
		void draw( );
	private:
		::sequence::Seq* const mNextSequence;
		sf::RenderWindow& mWindow;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}