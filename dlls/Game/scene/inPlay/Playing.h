#pragma once
#pragma hdrstop
#include <Lib/precompiled.h>
#include "IScene.h"
#include "../../model/Tetrimino.h"
#include "../../model/Stage.h"

namespace scene::inPlay
{
	class Playing final : public ::scene::inPlay::IScene
	{
	public:
		Playing( ) = delete;
		Playing( sf::RenderWindow& window, sf::Drawable& shapeOrSprite );
		~Playing( ) = default;

		void loadResources( ) override;
		void update( ::scene::inPlay::IScene** const nextScene, std::queue< sf::Event >& eventQueue ) override;
		void draw( ) override;
	private:
		uint16_t mFrameCount;
		sf::RenderWindow& mWindow_;
		sf::RectangleShape& mBackgroundRect_;
		::model::Tetrimino mCurrentTetrimino;
		::model::Stage mPlayerStage;
		//sf::RectangleShape mOpponentPanel;
	};
}