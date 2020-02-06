#pragma once
#include "../IScene.h"
#include "IScene.h"

namespace scene::inPlay
{
	class InPlay final : public ::scene::IScene
	{
	public:
		InPlay( sf::RenderWindow& window );
		~InPlay( );

		void loadResources( ) override;
		::scene::ID update( std::vector<sf::Event>& eventQueue ) override;
		void draw( ) override;
#ifdef _DEV
		::scene::ID currentScene( ) const override;
#endif
	private:
		void setScene( const ::scene::inPlay::ID nextInPlaySceneID );
		static bool IsInstantiated;
		uint32_t mFPS_;
		sf::RenderWindow& mWindow_;
		std::unique_ptr< ::scene::inPlay::IScene > mCurrentScene;
		std::unique_ptr< ::scene::inPlay::IScene > mOverlappedScene;
		sf::RectangleShape mBackground;
	};
}