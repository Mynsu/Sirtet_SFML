#pragma once
#pragma hdrstop
#include "../../Common.h"
#include "../ISequence.h"

namespace sequence::inPlay
{
	enum class Seq
	{
		READY = 0,
		NONE,
	};

	class GAME_API InPlay final : public ISequence
	{
	public:
		InPlay( ) = delete;
		InPlay( sf::RenderWindow& window,
			  ::sequence::Seq* nextSequence );
		~InPlay( ) = default;

		void update( ) override;
		void draw( ) override;
	private:
		sf::RenderWindow& const mWindow;
		std::unique_ptr< ::sequence::ISequence > mCurrentSequence;
		//Seq mNextSequence;
	};
}