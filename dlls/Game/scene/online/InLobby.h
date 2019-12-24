#pragma once
#include "IScene.h"

const uint8_t MOVING_POINT_NUM = 4;

namespace scene::online
{
	class Online;
}

namespace scene::online
{
	class InLobby final : public ::scene::online::IScene
	{
	public:
		InLobby( ) = delete;
		InLobby( sf::RenderWindow& window, ::scene::online::Online& net );
		~InLobby( );

		void loadResources( ) override;
		::scene::online::ID update( std::list<sf::Event>& eventQueue ) override;
		void draw( ) override;
	private:
		void cancelConnection( const std::string_view& );
		void createRoom( const std::string_view& );
		void joinRoom( const std::string_view& arg );
		static bool IsInstantiated, BinarySemaphore;
		bool mIsReceiving, mHasCanceled, mHasJoined;
		uint32_t mFrameCount_update;
		::scene::online::Online& mNet;
		sf::RenderWindow& mWindow_;
		std::array< sf::Vector2f, MOVING_POINT_NUM > mMovingPoint;
		std::unordered_map< std::string, std::pair<sf::Text, uint8_t> > mUserList;
		sf::Font mFont_;
		sf::RectangleShape mBackground;
		sf::RectangleShape mUserNicknamesBox;
	};
}