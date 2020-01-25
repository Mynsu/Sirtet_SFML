#pragma once
#include "IScene.h"
#include "../../ui/PlayView.h"
#include "../../model/Tetrimino.h"

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
		struct
		{
			float cellSize, outlineThickness_on, angularVelocity, arcLength, scaleFactor;
			uint32_t panelColor_on, outlineColor_on, framesRotationInterval,
				myNicknameFontSize, myNicknameColor,
				otherPlayerNicknameFontSize, otherPlayerNicknameFontColor;
			sf::Vector2f position;
			sf::Vector2f positionDifferences[ROOM_CAPACITY-1];
			sf::Vector2i countdownSpriteClipSize;
			std::string countdownSpritePathNName;
		} mDrawingInfo;
		void startGame( );
		void _startGame( const std::string_view& );
		void leaveRoom( );
		void _leaveRoom( const std::string_view& );
		static bool IsInstantiated;
		bool mIsReceiving, mAsHost, mIsPlaying,
			mIsMouseOverStartButton_, mIsStartButtonPressed_;
		sf::RenderWindow& mWindow_;
		uint32_t mFrameCount_rotationInterval;
		Online& mNet;
		sf::FloatRect mNextTetriminoPanelBound;
		// When empty, the value should be 0 == EMPTY_SLOT.
		HashedKey mOtherPlayerSlots[ROOM_CAPACITY-1];
		sf::Font mFont;
		sf::Text mStartingGuide, mNicknameLabel;
		std::unordered_map<HashedKey, ::scene::online::Participant> mParticipants;
		sf::RectangleShape mBackground;
		sf::RectangleShape mOtherPlayerSlotBackground;
	};
}