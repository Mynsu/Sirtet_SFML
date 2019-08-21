#pragma once
#pragma hdrstop
#include <Lib/precompiled.h>
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
		void update( std::queue< sf::Event >& ) override;
		void draw( ) override;
		::scene::ID currentScene( ) const override;
		///auto newEqualTypeInstance( ) -> std::unique_ptr< ::scene::IScene > override;
	private:
		// Only a single instance for a type can live at a time, but shouldn't be accessible globally.
		// That's the difference from the class filled with static functions, or on singleton pattern and the like.
		static bool IsInstantiated;
		sf::RenderWindow& mWindow;
		const SetScene_t& mSetScene;
		::scene::ID mOnIndicator;
		sf::Vector2< uint16_t > mSpriteClipSize;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}