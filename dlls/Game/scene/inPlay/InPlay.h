#pragma once
#include "../IScene.h"
#include "IScene.h"

namespace scene::inPlay
{
	class InPlay final : public ::scene::IScene
	{
	public:
		InPlay( sf::RenderWindow& window,
			   const ::scene::inPlay::ID initScene = ::scene::inPlay::ID::READY );
		~InPlay( );

		void loadResources( ) override;
		::scene::ID update( std::vector<sf::Event>& eventQueue ) override;
		void draw( ) override;
		::scene::ID currentScene( ) const override;
		void setScene( const uint8_t sceneID ) override
		{
			setScene( (::scene::inPlay::ID)sceneID );
		}
	private:
		void setScene( const ::scene::inPlay::ID nextInPlaySceneID );
		static bool IsInstantiated;
		uint16_t mFPS_;
		sf::RenderWindow& mWindow_;
		std::unique_ptr<::scene::inPlay::IScene> mCurrentScene;
		std::unique_ptr<::scene::inPlay::IScene> mOverlappedScene;
		sf::RectangleShape mBackground;
	};
}