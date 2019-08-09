#pragma once
#pragma hdrstop
#include "../Common.h"
#include "ISequence.h"

namespace sequence
{
	class Intro final : public ::sequence::ISequence
	{
	public:
		Intro( ) = delete;
		Intro( sf::RenderWindow& window, const SetSequence_t& setSequence );
		~Intro( );

		void update( ) override;
		void draw( ) override;
		auto newEqualTypeInstance( ) -> std::unique_ptr< ::sequence::ISequence > override;
	private:
		// Time unit: Seconds.
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
		uint16_t mFrameCount;
		uint16_t mFPS;
		sf::RenderWindow& mWindow;
		const SetSequence_t& mSetSequence;
		::sequence::Seq mNextSequence;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}
