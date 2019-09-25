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
		///bool mHasConnectionFailed_;
		uint32_t mFPS_, mFrameCount_disconnection;
		sf::Vector2f mSpriteClipSize_;
		sf::RenderWindow& mWindow_;
		const SetScene_t& mSetScene;
		Socket& mSocket_;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}