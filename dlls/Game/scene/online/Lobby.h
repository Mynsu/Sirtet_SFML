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
		uint32_t mFPS_, mFrameCount_disconnection;
		int32_t mQueueNumber;
		sf::Vector2f mSpriteClipSize_;
		sf::RenderWindow& mWindow_;
		const SetScene_t& mSetScene;
		std::unique_ptr< Socket > mSocketToQueueServer;
		std::unique_ptr< Socket > mSocketToMainServer;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}