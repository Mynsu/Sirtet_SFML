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
		struct DrawingInfo
		{
			float cellSize, outlineThickness_on;
			uint32_t panelColor, outlineColor_on;
			sf::Vector2f position;
			sf::Vector2f positionDifferences[ROOM_CAPACITY-1];
			sf::Vector2i countdownSpriteClipSize;
			std::string countdownSpritePathNName;
			sf::Text nicknameLabel_;
		};
		void startGame( const std::string_view& arg );
		void leaveRoom( const std::string_view& arg );
		static bool IsInstantiated;
		bool mIsReceiving, mHasCanceled, mIsPlaying_, mAsHost_;
		sf::RenderWindow& mWindow_;
		Online& mNet;
		// When empty, this contains 0 == EMPTY_SLOT.
		HashedKey mOtherPlayerSlots[ROOM_CAPACITY-1];
		DrawingInfo mDrawingInfo;
		sf::Font mFont;
		std::unordered_map<HashedKey, ::scene::online::Participant> mParticipants;
		sf::RectangleShape mBackground;
		sf::RectangleShape mOtherPlayerSlotBackground;
	};
}