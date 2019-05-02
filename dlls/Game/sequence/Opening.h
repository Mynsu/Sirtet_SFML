#pragma once
#include "../Common.h"
#include <SFML/Graphics.hpp>
#include "ISequence.h"

namespace sequence
{
	class GAME_API Opening final : public ISequence
	{
	public:
		Opening( ) = delete;
		Opening( const sf::RenderWindow& window );
		~Opening( ) = default;

		void update( ) override;
		void draw( ) override;
	private:
		const sf::RenderWindow& mWindow;
		sf::Texture mTexture;
	};
}
