#pragma once
#pragma hdrstop
#include <Lib/precompiled.h>
#include "../IScene.h"
#include "IScene.h"

namespace scene::inPlay
{
	class InPlay final : public ::scene::IScene
	{
	public:
		InPlay( ) = delete;
		InPlay( sf::RenderWindow& window, const SetScene_t& setScene );
		~InPlay( );

		void loadResources( ) override;
		void update( std::queue< sf::Event >& eventQueue ) override;
		void draw( ) override;
		::scene::ID currentScene( ) const override;
	private:
		static bool IsInstantiated;
		uint32_t mFPS;
		uint32_t mFrameCount;
		sf::RenderWindow& mWindow_;
		const SetScene_t& mSetScene_;
		std::unique_ptr< ::scene::inPlay::IScene > mCurrentScene;
		sf::RectangleShape mBackgroundRect;
	};
}