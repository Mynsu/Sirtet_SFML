#pragma once
#include "../model/Tetrimino.h"

namespace scene::online
{
	class Online;
}

namespace ui
{
	class PlayView
	{
	public:
		// NOTE: DO NOT USE.  Declared to use std::unordered_map.
		PlayView( );
		PlayView( sf::RenderWindow& window, ::scene::online::Online& net );
		~PlayView( ) = default;
		
		void setDimension( const sf::Vector2f position, const float cellSize );
		void update( );
		void draw( const int time );
	private:
		bool mHasStarted;
		float mCellSize;
		sf::RenderWindow* mWindow_;
		::scene::online::Online* mNet;
		sf::Texture mTexture;
		sf::Sprite mSprite;
		::model::Tetrimino mCurrentTetrimino;
		::model::Stage mStage;
	};
}