#pragma once
#include "IScene.h"

namespace scene::inPlay
{
	class Assertion : public ::scene::inPlay::IScene
	{
	public:
		Assertion( ) = delete;
		Assertion( sf::RenderWindow& window, bool* isESCPressed );
		~Assertion( ) = default;
		void loadResources( ) override;
		int8_t update( ::scene::inPlay::IScene** const, std::list< sf::Event >& eventQueue ) override;
		void draw( ) override;
	private:
		uint32_t mFPS_, mFrameCount;
		bool* mIsESCPressed;
		sf::RenderWindow& mWindow_;
		sf::RectangleShape mRect;
	};
}