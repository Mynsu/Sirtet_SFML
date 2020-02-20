#pragma once
#include "IScene.h"

namespace scene::inPlay
{
	class Assertion final : public ::scene::inPlay::IScene
	{
	public:
		Assertion( sf::RenderWindow& window );
		~Assertion( ) = default;

		void loadResources( ) override;
		::scene::inPlay::ID update( std::vector<sf::Event>& eventQueue ) override;
		void draw( ) override;
	private:
		enum class SoundIndex
		{
			ON_SELECTION,
			NULL_MAX,
		};
		uint16_t mFPS_, mFrameCountToCancel;
		sf::RenderWindow& mWindow_;
		std::string mSoundPaths[(int)SoundIndex::NULL_MAX];
		sf::Font mFont;
		sf::Text mTextLabelForGuide;
		sf::RectangleShape mBackground;
	};
}