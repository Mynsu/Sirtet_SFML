#pragma once
#include "../IScene.h"

namespace scene::online
{
	class Lobby : public ::scene::IScene
	{
	public:
		Lobby( ) = delete;
		Lobby( sf::RenderWindow& window, const SetScene_t& setScene );
		~Lobby( );

		virtual void loadResources( ) override;
		virtual void update( std::list<sf::Event>& eventQueue ) override;
		virtual void draw( ) override;
		virtual ::scene::ID currentScene( ) const override;
	private:
		static bool IsInstantiated;
//TODO: Thread2 != nullptr로 대체할 수 있을지도.
		bool mIsWaitingForTicketVerification;
		// When 0, frame counters don't increase. While 1 or more, they increase.
		// In other words, setting the value 0 to 1( or any number except 0 ) acts like a trigger.
		uint32_t mFPS_, mFrameCount_disconnection, mFrameCount_receivingInterval_;
		int32_t mMyOrderInQueueLine;
		sf::Vector2f mSpriteClipSize_;
		sf::RenderWindow& mWindow_;
		const SetScene_t& mSetScene;
		std::unique_ptr< Socket > mSocketToQueueServer;
		std::unique_ptr< Socket > mSocketToMainServer;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}