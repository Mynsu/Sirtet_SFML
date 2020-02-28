#pragma once
#include "IScene.h"

namespace scene::inPlay
{
	class Assertion final : public ::scene::inPlay::IScene
	{
	private:
		static bool IsInstantiated;
	public:
		Assertion( sf::RenderWindow& window );
		~Assertion( );

		void loadResources( sf::RenderWindow& window ) override;
		::scene::inPlay::ID update( std::vector<sf::Event>& eventQueue ) override;
		void draw( sf::RenderWindow& window ) override;
	private:
		enum class SoundIndex
		{
			SELECTION,
			NULL_MAX,
		};
		uint16_t mFPS_, mFrameCountToCancel;
		std::string mSoundPaths[(int)SoundIndex::NULL_MAX];
		sf::Font mFont;
		sf::Text mTextLabelForGuide;
		sf::RectangleShape mBackground;
	};
}