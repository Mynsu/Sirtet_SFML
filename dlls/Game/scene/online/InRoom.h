#pragma once
#include "IScene.h"
#include "../../ui/StageView.h"
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
		//void invite( const std::string_view& arg );
		void startGame( const std::string_view& arg );
		void leaveRoom( const std::string_view& arg );
		bool mAsHost, mIsReceiving;
		HashedKey mDigestedNickname_;
		int32_t mFrameCount, mFPS_;
		sf::RenderWindow& mWindow_;
		Online& mNet;
//TODO
		std::unordered_map< HashedKey, ::ui::StageView > mParticipants;
		sf::RectangleShape mBackgroundRect;
	};
}