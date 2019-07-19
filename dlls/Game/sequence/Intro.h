#pragma once
#pragma hdrstop
#include "../Common.h"
#include "ISequence.h"

namespace sequence
{
	class GAME_API Intro final : public ::sequence::ISequence
	{
	public:
		Intro( ) = delete;
		Intro( sf::RenderWindow& window,
				 ::sequence::Seq* const nextMainSequence );
		~Intro( )
		{
			IsInstanciated = false;
		}

		void update( ) override;
		void draw( ) override;
	private:
		::sequence::Seq* const mNextMainSequence;
		sf::RenderWindow& mWindow;
		sf::Texture mTexture;
		sf::Sprite mSprite;
		// A single instance can live at a time, two or more can't.
		// NOTE: Singleton pattern and service locator pattern also give this,
		//		 but global access isn't necessary here.
		static bool IsInstanciated;
		// NOTE: 'uint8_t' and 'uint16_t' have been used just for saving memory, which are less than 'int.'
		uint8_t mDuration;
		// NOTE: It looks odd at first glance
		// that concept(or object) 'Intro' has an attribute(or property) 'alpha of RGBA.'
		// However, if a global or a symbol of alpha were in an anonymous namespace or static in member function,
		// that alpha must be manually zero-initialized with an instance of 'Intro' being destructed.
		// THIS IS NOT RAII.
		// Moreover, even while no instance of 'Intro' lives,
		// the symbol still lives.
		// THIS IS MEMORY WASTE.
		uint8_t mAlpha;
		// NOTE: Please look at the comment of 'mAlpha.'
		uint16_t mFrameCount;
	};
}
