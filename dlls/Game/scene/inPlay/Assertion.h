#pragma once
#include "IScene.h"

namespace scene::inPlay
{
	class Assertion final : public ::scene::inPlay::IScene
	{
	public:
		Assertion( ) = delete;
		Assertion( sf::RenderWindow& window );
		~Assertion( ) = default;
		void loadResources( ) override;
		::scene::inPlay::ID update( std::list<sf::Event>& eventQueue ) override;
		void draw( ) override;
	private:
		uint32_t mFPS_, mFrameCount;
		sf::RenderWindow& mWindow_;
		sf::Font mFont;
		sf::Text mGuideTextLabel;
		sf::RectangleShape mBackground;
	};
}