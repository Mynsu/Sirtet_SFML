#pragma once
#include "../IScene.h"
#include "IScene.h"

namespace scene::inPlay
{
	class InPlay final : public ::scene::IScene
	{
	public:
		InPlay( ) = delete;
		InPlay( sf::RenderWindow& window, const SetScene_t& setScene, const ::scene::ID mode = ::scene::ID::SINGLE_PLAY );
		~InPlay( );

		void loadResources( ) override;
		void update( std::list< sf::Event >& eventQueue ) override;
		void draw( ) override;
		::scene::ID currentScene( ) const override;
	private:
		static bool IsInstantiated;
		uint32_t mFPS;
		const ::scene::ID mMode;
		sf::RenderWindow& mWindow_;
		const SetScene_t& mSetScene_;
		std::unique_ptr< ::scene::inPlay::IScene > mCurrentScene;
		std::unique_ptr< ::scene::inPlay::IScene > mOverlappedScene;
		sf::RectangleShape mBackgroundRect;
	};
}