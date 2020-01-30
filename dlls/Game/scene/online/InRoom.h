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
		// !IMPORTANT: DO NOT USE!  Defined to use std::unordered_map.
		Participant() = default;
		Participant( const std::string& nickname, ::ui::PlayView&& playView )
			: nickname( nickname ), playView( std::forward<::ui::PlayView&&>(playView) )
		{}
		const std::string nickname;
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
		::scene::online::ID update( std::list<sf::Event>& eventQueue ) override;
		void draw( ) override;
	private:
		void startGame( ) const;
		void _startGame( const std::string_view& );
		void leaveRoom( ) const;
		void _leaveRoom( const std::string_view& );
		enum class AlarmIndex
		{
			ALL_OVER_FREEZE,
			NONE_MAX,
		};
		// Also reset.
		inline bool alarmAfter( const uint32_t milliseconds, const AlarmIndex index )
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
			uint32_t panelColor_on, outlineColor_on, framesRotationInterval,
				myNicknameFontSize, myNicknameColor,
				otherPlayerNicknameFontSize, otherPlayerNicknameFontColor;
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
		uint32_t mFrameCount_rotationInterval;
		Clock::time_point mAlarms[(int)AlarmIndex::NONE_MAX];
		Online& mNet;
		std::unique_ptr<::scene::inPlay::IScene> mOverlappedScene;
		// When empty, the value should be 0 == EMPTY_SLOT.
		HashedKey mOtherPlayerSlots[ROOM_CAPACITY-1];
		sf::Font mFont;
		sf::Text mStartingGuide, mNicknameLabel;
		std::unordered_map<HashedKey, ::scene::online::Participant> mParticipants;
		sf::RectangleShape mBackground;
		sf::RectangleShape mOtherPlayerSlotBackground;
	};
}