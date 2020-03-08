#pragma once
#include "IScene.h"
#include "../../ui/PlayView.h"
#include "../../model/Tetrimino.h"
#include "../inPlay/Assertion.h"

const uint8_t ROOM_CAPACITY = 4;

namespace scene::online
{
	struct Participant
	{
	public:
		Participant( const std::string& nickname, const bool isPlayable );
		Participant( const Participant& ) = delete;
		Participant( Participant&& ) = delete;
		void operator=( const Participant& ) = delete;
		~Participant() = default;
			
		std::string nickname;
		::ui::PlayView playView;
	};

	class Online;

	class InRoom final : public ::scene::online::IScene
	{
	private:
		// 둘 이상의 인스턴스를 만들 수 없습니다.
		static bool IsInstantiated;
	public:
		InRoom( ) = delete;
		InRoom( const sf::RenderWindow& window, Online& net, const bool asHost = false );
		~InRoom( );

		void loadResources( const sf::RenderWindow& window ) override;
		::scene::online::ID update( std::vector<sf::Event>& eventQueue,
								   ::scene::online::Online& net,
								   const sf::RenderWindow& window ) override;
		void draw( sf::RenderWindow& window ) override;
	private:
		enum class AlarmIndex
		{
			ALL_OVER_FREEZE,
			NONE_MAX,
		};
		enum class SoundIndex
		{
			SELECTION,
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
				mAlarms[(int)index] = (Clock::time_point::max)();
				retVal = true;
			}
			return retVal;
		}
		struct
		{
			uint16_t framesRotationInterval,
				myNicknameFontSize, otherPlayerNicknameFontSize;
			uint32_t cellOutlineColor, panelColor_on, outlineColor_on,
				myNicknameColor, otherPlayerNicknameFontColor;
			float cellSize, outlineThickness_on, angularVelocity, arcLength, scaleFactor;
			sf::Vector2f position, positionDifferences[ROOM_CAPACITY-1];
			sf::Vector2i countdownSpriteClipSize;
			sf::FloatRect nextTetriminoPanelBound;
			std::string countdownSpritePath;
		} mDrawingInfo;
		bool mIsReceiving, mAsHost,
			mIsStartingGuideVisible, mIsMouseOverStartButton_, mIsStartButtonPressed_;
		uint32_t mFrameCountCoolToRotateStartButton;
		const HashedKey mMyNicknameHashed_;
		const std::string& mMyNickname;
		// Recommended to use only for console command like startGame(...), leaveGame(...).
		::scene::online::Online& mNet;
		Clock::time_point mAlarms[(int)AlarmIndex::NONE_MAX];
		std::unique_ptr<::scene::inPlay::IScene> mOverlappedScene;
		// When empty, the value should be 0 == NULL_EMPTY_SLOT.
		HashedKey mOtherPlayerSlots[ROOM_CAPACITY-1];
		std::string mSoundPaths[(int)SoundIndex::NONE_MAX];
		sf::Font mFont;
		sf::Text mTextLabelForStartingGuide, mLabelForNickname;
		std::unordered_map<HashedKey, Participant> mParticipants;
		sf::RectangleShape mBackground;
		sf::RectangleShape mOtherPlayerSlotBackground;
	};
}