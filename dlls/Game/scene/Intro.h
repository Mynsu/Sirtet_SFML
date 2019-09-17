#pragma once
#pragma hdrstop
#include <Lib/precompiled.h>
#include "IScene.h"

namespace scene
{
	class Intro final : public ::scene::IScene
	{
	public:
		Intro( ) = delete;
		Intro( sf::RenderWindow& window, const SetScene_t& setScene );
		~Intro( );

		void loadResources( ) override;
		///void update( std::queue< sf::Event >& ) override;
		void update( std::vector< sf::Event >& ) override;
		void draw( ) override;
		::scene::ID currentScene( ) const override;
	private:
		// Only a single instance for a type can live at a time, but shouldn't be accessible globally.
		// That's the difference from the class filled with static functions, or on singleton pattern and the like.
		static bool IsInstantiated;
		// Time Unit: Seconds.
		const uint8_t mDuration;
		uint8_t mAlpha_;
		int32_t mFrameCount;
		uint32_t mFPS_;
		sf::RenderWindow& mWindow_;
		const SetScene_t& mSetScene_;
		::scene::ID mNextScene_;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}
