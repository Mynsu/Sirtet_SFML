#pragma once
#include "IScene.h"

namespace scene::online
{
	class Online;

	class Waiting final : public ::scene::online::IScene
	{
	public:
		Waiting( sf::RenderWindow& window, Online& net );
		~Waiting( );
		
		void loadResources( ) override;
		::scene::online::ID update( std::list<sf::Event>& eventQueue ) override;
		void draw( ) override;
	private:
		enum class State
		{
			TICKETING,
			SUBMITTING_TICKET,
		};
		static bool IsInstantiated;
		uint32_t mOrder;
		::scene::online::Waiting::State mState;
		sf::RenderWindow& mWindow_;
		Online& mNet;
		sf::Font mFont;
		sf::Text mTextLabels[3];
	};
}