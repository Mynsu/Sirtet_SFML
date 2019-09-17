#pragma once
#pragma hdrstop
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
		int8_t update( ::scene::inPlay::IScene** const, std::vector< sf::Event >& eventQueue ) override;
		void draw( ) override;
	private:
		uint32_t mFPS, mFrameCount;
		bool* mIsESCPressed;
		sf::RenderWindow& mWindow;
		sf::RectangleShape mRect;
	};
}