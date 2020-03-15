#pragma once
#include "../IScene.h"
#include "IScene.h"

namespace scene::inPlay
{
	class InPlay final : public ::scene::IScene
	{
	private:
		static bool IsInstantiated;
	public:
		InPlay( const sf::RenderWindow& window,
			   const ::scene::inPlay::ID initScene = ::scene::inPlay::ID::READY );
		~InPlay( );

		void loadResources( const sf::RenderWindow& window ) override;
		::scene::ID update( std::vector<sf::Event>& eventQueue,
						   const sf::RenderWindow& window ) override;
		void draw( sf::RenderWindow& window ) override;
		::scene::ID currentScene( ) const override;
		void setScene( const uint8_t sceneID,
					  const sf::RenderWindow& window ) override
		{
			setScene( (::scene::inPlay::ID)sceneID, window );
		}
	private:
		void setScene( const ::scene::inPlay::ID nextInPlaySceneID,
					  const sf::RenderWindow& window );
		uint16_t mFPS_;
		std::unique_ptr<::scene::inPlay::IScene> mCurrentScene;
		// µ¿½Ã¿¡ ¿©·¯ ¾ÀÀ» ¶ç¿ó´Ï´Ù.
		std::unique_ptr<::scene::inPlay::IScene> mOverlappedScene;
		sf::RectangleShape mBackground;
	};
}