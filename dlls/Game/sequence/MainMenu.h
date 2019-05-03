#pragma once
#pragma hdrstop
#include "../Common.h"
#include "ISequence.h"

namespace sequence
{
	class MainMenu final : public ISequence
	{
	public:
		MainMenu( ) = delete;
		MainMenu( sf::RenderWindow& window );
		~MainMenu( ) = default;

		void update( );
		void draw( );
	private:
		sf::RenderWindow& mWindow;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}