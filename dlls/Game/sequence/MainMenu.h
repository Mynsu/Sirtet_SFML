#pragma once
#pragma hdrstop
#include "../Common.h"
#include "ISequence.h"

namespace sequence
{
	class MainMenu final : public ::sequence::ISequence
	{
	public:
		MainMenu( ) = delete;
		MainMenu( sf::RenderWindow& window, const SetSequence_t& setSequence );
		~MainMenu( );

		void update( ) override;
		void draw( ) override;
		auto newEqualTypeInstance( ) -> std::unique_ptr< ::sequence::ISequence > override;
	private:
		::sequence::Seq mOnIndicator;
		sf::RenderWindow& mWindow;
		const SetSequence_t& mSetSequence;
		sf::Vector2u mSpriteClipSize;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}