#pragma once
#pragma hdrstop
#include "ISequence.h"

namespace sequence::inPlay
{
	class Playing final : public ::sequence::inPlay::ISequence
	{
	public:
		Playing( ) = delete;
		Playing( sf::RenderWindow& window, ::sequence::inPlay::Seq* const nextInPlaySequence, sf::Drawable& shapeOrSprite );
		~Playing( ) = default;

		void update( ) { }
		void draw( );
	private:
		sf::RenderWindow& mWindow;
		::sequence::inPlay::Seq* const mNextInPlaySequence;
		sf::RectangleShape& mBackgroundRect;
		sf::RectangleShape mPlayerPanel;
		sf::RectangleShape mOpponentPanel;
	};
}