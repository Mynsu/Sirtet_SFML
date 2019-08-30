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
		int32_t mFrameCount0_, mFrameCount1_, mFrameCount2_;
		float mTempo;
		float mCellSize_;
		sf::RenderWindow& mWindow_;
		sf::RectangleShape& mBackgroundRect_;
		sf::Vector2f mMargin_;
		sf::Vector2f mNextTetriminoPanelPosition_;
		sf::RectangleShape mNextTetriminoPanel;
		sf::RectangleShape mNextTetriminoBlock_;
		::vfx::Combo mVfxCombo;
		::model::Tetrimino mCurrentTetrimino;
		std::queue< ::model::Tetrimino > mNextTetriminos;
		::model::Stage mPlayerStage;

		//sf::RectangleShape mOpponentPanel;
	};
}