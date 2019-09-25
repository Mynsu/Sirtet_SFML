#pragma once
#include "IScene.h"

namespace scene
{
	class MainMenu final : public ::scene::IScene
	{
	public:
		MainMenu( ) = delete;
		MainMenu( sf::RenderWindow& window, const SetScene_t& setScene );
		~MainMenu( );

		void loadResources( ) override;
		void update( std::list< sf::Event >& eventQueue ) override;
		void draw( ) override;
		::scene::ID currentScene( ) const override;
	private:
		void touchButton( );

		// Only a single instance for a type can live at a time, but shouldn't be accessible globally.
		// That's the difference from the class filled with static functions, or on singleton pattern and the like.
		static bool IsInstantiated;
		sf::RenderWindow& mWindow_;
		const SetScene_t& mSetScene;
		::scene::ID mOnIndicator;
		sf::Vector2f mSpriteClipSize_;
		sf::Vector2f mLogoMargin_;
		sf::Vector2f mButtonSinglePosition_, mButtonOnlinePosition_;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}