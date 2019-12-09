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
		bool update( std::list<sf::Event>& eventQueue );
		void draw( const int time );
	private:
		bool mHasTetrimino, mHasTetriminoCollidedInClient;
		// NOTE: Also as mHasTetriminoCollidedIn-Server.
		uint32_t mFrameCount_sync;
		uint32_t mFrameCount_falldown, mFrameCount_input;
		uint32_t mTempo;
		float mCellSize;
		sf::RenderWindow* mWindow_;
		::scene::online::Online* mNet;
		sf::Texture mTexture;
		sf::Sprite mSprite;
		::model::Tetrimino mCurrentTetrimino;
		std::string mNextTetrimino, mNextStage;
		::model::Stage mStage;
	};
}