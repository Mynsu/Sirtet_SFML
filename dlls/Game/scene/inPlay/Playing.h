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
	private:
		static bool IsInstantiated;
	public:
		Playing( sf::RenderWindow& window,
				sf::Drawable& shapeOrSprite,
				const std::unique_ptr<::scene::inPlay::IScene>& overlappedScene );
		~Playing( );

		void loadResources( sf::RenderWindow& window ) override;
		::scene::inPlay::ID update( std::vector<sf::Event>& eventQueue ) override;
		void draw( sf::RenderWindow& window ) override;
	private:
		enum class StateAfterCooling
		{
			TO_GAME_OVER,
			TO_NEXT_LEVEL,
			TO_ALL_CLEARED,
			NONE,
		};
		enum class SoundIndex
		{
			TETRIMINO_LOCKED,
			LINE_CLEARED,
			LEVEL_CLEARED,
			NONE_MAX,
		};
		struct Mission
		{
			uint8_t numOfLinesToClear;
			float tempoOnStart;
		};
		void reloadTetrimino( );
		uint8_t mNumOfLinesRecentlyCleared, mCurrentLevel;
		int8_t mNumOfLinesRemainingToLevelClear;
		uint16_t mFrameCountSoftDropInterval, mFrameCountCool;
		// Makes it possible to not check out line clearing every frame.  That is needless.
		uint16_t mFrameCountClearingInterval_;
		// This also serves as flag.  When this isn't equal to 0, visual effect turns on and plays.
		uint16_t mFrameCountVfxDuration;
		// For units digit.
		int16_t mAnimationDamperForScore1;
		// For tens digit.
		int16_t mAnimationDamperScore10;
		StateAfterCooling mStateAfterCooling;
		struct
		{
			uint32_t blackOutColor;
			float cellSize, gapBetweenScoreLetter, animationSpeed;
			sf::Vector2i scoreSpriteClipSize;
			sf::Vector2f scorePosition;
		} mDrawingInfo;
		// Interval between soft drops.  0.0f ~ 1.0f.
		float mTempo;
		// NOTE: 읽기 쉽게 멤버 함수 draw(...)의 인자로 넘기려니
		// 오버라이드하고 있는 순수 가상 함수까지 바꿔야해서
		// 생성자로 레퍼런스를 받아 쥐고 있습니다.
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