#pragma once
#include "IScene.h"
#include "../../model/Tetrimino.h"
#include "../../model/Stage.h"
#include "../../vfx/Combo.h"
#include "../../ui/NextTetriminoPanel.h"

namespace scene::inPlay
{
	class Playing final : public ::scene::inPlay::IScene
	{
	public:
		Playing( sf::RenderWindow& window,
				sf::Drawable& shapeOrSprite,
				const std::unique_ptr<::scene::inPlay::IScene>& overlappedScene );
		~Playing( ) = default;

		void loadResources( ) override;
		::scene::inPlay::ID update( std::vector<sf::Event>& eventQueue ) override;
		void draw( ) override;
	private:
		enum class AudioIndex
		{
			TETRIMINO_LOCKED,
			LINE_CLEARED,
			NONE_MAX,
		};
		void reloadTetrimino( );
		struct
		{
			uint32_t blackOutColor;
			float cellSize_;
		} mDrawingInfo;
		uint8_t mNumOfLinesCleared;
		uint16_t mFrameCountSoftDropInterval, mFrameCountClearingInterval_,
			mFrameCountVfxDuration_, mFrameCountCoolToGameOver;
		float mTempo;
		sf::RenderWindow& mWindow_;
		sf::RectangleShape& mBackgroundRect_;
		const std::unique_ptr<::scene::inPlay::IScene>& mOverlappedScene_;
		std::string mAudioList[(int)AudioIndex::NONE_MAX];
		::ui::NextTetriminoPanel mNextTetriminoPanel;
		::vfx::Combo mVfxCombo;
		::model::Tetrimino mCurrentTetrimino;
		std::queue<::model::Tetrimino> mNextTetriminos;
		::model::Stage mStage;
	};
}