#pragma once
#include "IScene.h"
#include "../../ui/PlayView.h"
#include "../../model/Tetrimino.h"
#include "../inPlay/Assertion.h"

const uint8_t ROOM_CAPACITY = 4;

namespace scene::online
{
	struct Participant final
	{
		Participant( const std::string& nickname, ::ui::PlayView&& playView );
		void operator=( const Participant& ) = delete;
			
		std::string nickname;
		::ui::PlayView playView;
	};

	class Online;

	class InRoom final : public ::scene::online::IScene
	{
	public:
		InRoom( ) = delete;
		InRoom( sf::RenderWindow& window, Online& net, const bool asHost = false );
		~InRoom( );

		void loadResources( ) override;
		::scene::online::ID update( std::vector<sf::Event>& eventQueue ) override;
		void draw( ) override;
	private:
		enum class AlarmIndex
		{
			ALL_OVER_FREEZE,
			NONE_MAX,
		};
		enum class AudioIndex
		{
			ON_SELECTION,
			GAME_OVER,
			NONE_MAX,
		};
		void startGame( ) const;
		void _startGame( const std::string_view& );
		void leaveRoom( ) const;
		void _leaveRoom( const std::string_view& );
		bool alarmAfterAndReset( const uint16_t milliseconds, const AlarmIndex index )
		{
			bool retVal = false;
			if ( std::chrono::milliseconds(milliseconds) < Clock::now()-mAlarms[(int)index] )
			{
				mAlarms[(int)index] = Clock::time_point::max();
				retVal = true;
			}
			return retVal;
		}
		struct
		{
			uint16_t framesRotationInterval,
				myNicknameFontSize, otherPlayerNicknameFontSize;
			uint32_t panelColor_on, outlineColor_on,
				myNicknameColor, otherPlayerNicknameFontColor;
			float cellSize, outlineThickness_on, angularVelocity, arcLength, scaleFactor;
			sf::Vector2f position, positionDifferences[ROOM_CAPACITY-1];
			sf::Vector2i countdownSpriteClipSize;
			sf::FloatRect nextTetriminoPanelBound;
			std::string countdownSpritePathNName;
		} mDrawingInfo;
		static bool IsInstantiated;
		bool mIsReceiving, mAsHost,
			mIsStartingGuideVisible_, mIsMouseOverStartButton_, mIsStartButtonPressed_;
		sf::RenderWindow& mWindow_;
		uint32_t mFrameCountCoolToRotateStartButton;
		Clock::time_point mAlarms[(int)AlarmIndex::NONE_MAX];
		Online& mNet;
		std::unique_ptr<::scene::inPlay::IScene> mOverlappedScene;
		// When empty, the value should be 0 == EMPTY_SLOT.
		HashedKey mOtherPlayerSlots[ROOM_CAPACITY-1];
		std::string mAudioList[(int)AudioIndex::NONE_MAX];
		sf::Font mFont;
		sf::Text mTextLabelForStartingGuide, mLabelForNickname;
		std::unordered_map<HashedKey, ::scene::online::Participant> mParticipants;
		sf::RectangleShape mBackground;
		sf::RectangleShape mOtherPlayerSlotBackground;
	};
}