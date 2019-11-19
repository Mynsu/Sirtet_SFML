#pragma once
#include "../model/Tetrimino.h"

namespace ui
{
	class StageView
	{
	public:
		StageView( );
		~StageView( ) = default;
		void setDimension( const sf::Vector2f position, const float cellSize );
		void draw( sf::RenderWindow& window, const int time );

		void setCurrentTetrimino( const ::model::tetrimino::Info& info );
	private:
		bool mHasCurrentTetrimino;
		float mCellSize;
		///sf::RenderWindow& mWindow_;
		::model::Tetrimino mCurrentTetrimino;
		sf::RectangleShape mPanel;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}