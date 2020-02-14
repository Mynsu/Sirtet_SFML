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
		::scene::online::ID update( std::vector<sf::Event>& eventQueue ) override;
		void draw( ) override;
	private:
		enum class State
		{
			TICKETING,
			SUBMITTING_TICKET,
		};
		enum class AudioIndex
		{
			ON_SELECTION,
			NULL_MAX,
		};
		static bool IsInstantiated;
		uint16_t mOrder;
		::scene::online::Waiting::State mState;
		sf::RenderWindow& mWindow_;
		Online& mNet;
		std::string mAudioList[(int)AudioIndex::NULL_MAX];
		sf::Font mFont;
		sf::Text mTextLabels[3];
	};
}