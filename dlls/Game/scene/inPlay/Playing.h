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

		void loadResources( sf::RenderWindow& window ) override;
		::scene::inPlay::ID update( std::vector<sf::Event>& eventQueue ) override;
		void draw( sf::RenderWindow& window ) override;
	private:
		enum class SoundIndex
		{
			TETRIMINO_LOCKED,
			LINE_CLEARED,
			LEVEL_CLEARED,
			NONE_MAX,
		};
		void reloadTetrimino( );
		struct Mission
		{
			uint8_t numOfLinesToClear;
			float tempoOnStart;
		};
		struct
		{
			uint32_t blackOutColor;
			float cellSize_, gapBetweenScoreLetter, animationSpeed;
			sf::Vector2i scoreSpriteClipSize;
			sf::Vector2f scorePosition;
		} mDrawingInfo;
		uint8_t mNumOfLinesRecentlyCleared, mCurrentLevel;
		int8_t mNumOfLinesRemainingToLevelClear;
		uint16_t mFrameCountSoftDropInterval, mFrameCountClearingInterval_,
			mFrameCountVfxDuration_,
			mFrameCountCoolToGameOver, mFrameCountCoolToNextLevel, mFrameCountCoolAllLevelsCleared;
		int16_t mAnimationDamper1, mAnimationDamper10;
		float mTempo;
		sf::RectangleShape& mBackgroundRect_;
		const std::unique_ptr<::scene::inPlay::IScene>& mOverlappedScene_;
		// Missions to each level.
		std::vector<Mission> mMissions;
		std::string mSoundPaths[(int)SoundIndex::NONE_MAX];
		sf::Texture mTextureForScore;
		sf::Sprite mSpriteForScore;
		::ui::NextTetriminoPanel mNextTetriminoPanel;
		::vfx::Combo mVfxCombo;
		::model::Tetrimino mCurrentTetrimino;
		std::queue<::model::Tetrimino> mNextTetriminos;
		::model::Stage mStage;
	};
}