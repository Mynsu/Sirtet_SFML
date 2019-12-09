#pragma once
#include "IScene.h"
#include "../../ui/PlayView.h"
#include "../../model/Tetrimino.h"

namespace scene::online
{
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
		void startGame( const std::string_view& arg );
		void leaveRoom( const std::string_view& arg );
		static bool IsInstantiated;
		bool mAsHost, mIsReceiving, mHasCanceled;
		HashedKey mDigestedNickname_;
		int32_t mFrameCount, mFPS_;
		sf::RenderWindow& mWindow_;
		Online& mNet;
//TODO
		std::unordered_map< HashedKey, ::ui::PlayView > mParticipants;
		sf::RectangleShape mBackgroundRect;
	};
}