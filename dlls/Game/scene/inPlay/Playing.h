#pragma once
#pragma hdrstop
#include <Lib/precompiled.h>
#include "IScene.h"

namespace scene::inPlay
{
	class Playing final : public ::scene::inPlay::IScene
	{
	public:
		Playing( ) = delete;
		Playing( sf::RenderWindow& window, sf::Drawable& shapeOrSprite );
		~Playing( ) = default;

		void loadResources( ) override;
		void update( std::unique_ptr< ::scene::inPlay::IScene >* const currentScene ) override;
		void draw( ) override;
	private:
		sf::RenderWindow& mWindow;
		sf::RectangleShape& mBackgroundRect;
		sf::RectangleShape mPlayerPanel;
		sf::RectangleShape mOpponentPanel;
	};
}