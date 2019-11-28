#pragma once
#include "../model/Tetrimino.h"

namespace ui
{
	class PlayView
	{
	public:
		// NOTE: DO NOT USE.  Declared to use std::unordered_map.
		PlayView( );
		PlayView( sf::RenderWindow& window );
		~PlayView( ) = default;
		void setDimension( const sf::Vector2f position, const float cellSize );
		void draw( const int time );

		void updateCurrentTetrimino( const std::string& data );
		void updateStage( const std::string& data );
	private:
		bool mHasCurrentTetrimino;
		float mCellSize;
		sf::RenderWindow* mWindow_;
		sf::Texture mTexture;
		sf::Sprite mSprite;
		sf::RectangleShape mPanel;
		::model::Tetrimino mCurrentTetrimino;
		::model::Stage mStage;
	};
}