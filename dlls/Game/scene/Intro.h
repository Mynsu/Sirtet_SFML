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
		void update( std::queue< sf::Event >& ) override;
		void draw( ) override;
		::scene::ID currentScene( ) const override;
		///auto newEqualTypeInstance( ) -> std::unique_ptr< ::scene::IScene > override;
	private:
		// Only a single instance for a type can live at a time, but shouldn't be accessible globally.
		// That's the difference from the class filled with static functions, or on singleton pattern and the like.
		static bool IsInstantiated;
		//
		// NOTE: 'uint8_t' and 'uint16_t' have been used just for saving memory, which are less than 'int.'
		//
		// Time Unit: Seconds.
		const uint8_t mDuration;
		uint8_t mAlpha_;
		uint16_t mFrameCount;
		uint16_t mFPS_;
		sf::RenderWindow& mWindow_;
		const SetScene_t& mSetScene_;
		::scene::ID mNextScene_;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}
