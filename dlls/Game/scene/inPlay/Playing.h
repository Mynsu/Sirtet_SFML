#pragma once
#pragma hdrstop
#include <queue>
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
		int8_t update( ::scene::inPlay::IScene** const nextScene, std::vector< sf::Event >& eventQueue ) override;
		void draw( ) override;
	private:
		bool mIsESCPressed;
		uint8_t mRowCleared;
		int32_t mFrameCount_fallDown_, mFrameCount_clearingInterval_, mFrameCount_clearingVfx_;
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