#pragma once
#include "IScene.h"

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
		//TODO: 방제, 비번 등
		void createRoom( const std::string_view& );
		void respondYes( const std::string_view& );
		void respondNo( const std::string_view& );
		bool mIsReceiving, mHasCanceled, mHasJoined;
		::scene::online::Online& mNet;
		sf::RenderWindow& mWindow_;
	};
}