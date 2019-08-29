#pragma once
#pragma hdrstop
#include <Lib/precompiled.h>
#include "IScene.h"
#include "../../model/Tetrimino.h"
#include "../../model/Stage.h"
#include "../../vfx/Combo.h"

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
		uint8_t mLineCleared;
		int32_t mFrameCount0, mFrameCount1, mFrameCount2;
		sf::RenderWindow& mWindow_;
		sf::RectangleShape& mBackgroundRect_;
		::model::Tetrimino mCurrentTetrimino;
		::model::Stage mPlayerStage;
		::vfx::Combo mVfxCombo;
		//sf::RectangleShape mOpponentPanel;
	};
}