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
		~Opening( ) = default;

		void update( ) override;
		void draw( ) override;
	private:
		::sequence::Seq* mNextSequence;
		sf::RenderWindow& mWindow;
		sf::Texture mTexture;//TODO 패딩 없애기
		sf::Sprite mSprite;
	};
}
