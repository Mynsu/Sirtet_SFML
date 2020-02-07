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
		enum class AudioIndex
		{
			ON_SELECTION,
			NULL_MAX,
		};
		uint32_t mFPS_, mFrameCount;
		sf::RenderWindow& mWindow_;
		std::string mAudioList[(int)AudioIndex::NULL_MAX];
		sf::Font mFont;
		sf::Text mGuideTextLabel;
		sf::RectangleShape mBackground;
	};
}