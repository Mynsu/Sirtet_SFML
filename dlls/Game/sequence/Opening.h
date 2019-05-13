#pragma once
#pragma hdrstop
#include "../Common.h"
#include "ISequence.h"

namespace sequence
{
	class GAME_API Opening final : public ISequence
	{
	public:
		Opening( ) = delete;
		Opening( sf::RenderWindow& window,
				 ::sequence::Seq* nextSequence );
		~Opening( )
		{
			IsInstanciated = false;
		}

		void update( ) override;
		void draw( ) override;
	private:
		// Only single instance can live, two or more can't.
		static bool IsInstanciated;
		// NOTE: 'uint8_t' and 'uint16_t' have been used just for saving memory, rather than 'int.'
		uint8_t mDuration;
		// NOTE: It doesn't seem to make sense at first glance
		// that concept(or an object) 'Opening' has an attribute of alpha of RGBA.
		// However, if an value of alpha were in an anonymous namespace or static in member function,
		// that alpha should be zero when an instance of 'Opening' has been destructed.
		// THIS IS NOT RAII.
		// Moreover, even when no instance of 'Opening' exists,
		// a global in an anonymous namespace or member function still lives.
		// THIS IS MEMORY WASTE.
		uint8_t mAlpha;
		// NOTE: Please look at the comment of 'mAlpha'
		uint16_t mFrameCount;
		::sequence::Seq* const mNextSequence;
		sf::RenderWindow& const mWindow;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}
