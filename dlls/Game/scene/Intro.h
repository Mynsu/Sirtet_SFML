#pragma once
#include "IScene.h"

namespace scene
{
	class Intro final : public ::scene::IScene
	{
	public:
		Intro( ) = delete;
		Intro( sf::RenderWindow& window );
		~Intro( );

		void loadResources( ) override;
		::scene::ID update( std::list< sf::Event >& ignored_eventQueue ) override;
		void draw( ) override;
#ifdef _DEV
		::scene::ID currentScene( ) const override;
#endif
	private:
		// Only a single instance for a type can live at a time, but shouldn't be accessible globally.
		// That's the difference from the class filled with static functions, or on singleton pattern and the like.
		static bool IsInstantiated;
		// Time Unit: Seconds.
		const uint8_t mDuration;
		uint8_t mAlpha_;
		uint32_t mFPS_, mFrameCount;
		sf::RenderWindow& mWindow_;
		::scene::ID mNextScene_;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}
