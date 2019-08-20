#pragma once
#pragma hdrstop
#include <Lib/precompiled.h>
#include "IScene.h"

namespace scene::inPlay
{
	enum class TetriminoShape
	{
		I,
		J, L,
		N, S,
		T,
		O,

		NONE_MAX,
	};

	// 4 x 4
	using LocalSpace = uint16_t;
	struct Tetrimino
	{
		uint8_t rotationID;
		LocalSpace rotation[ 4 ];
		// On global coordinate.
		// Unit: Grid.
		sf::Vector2u position;
	};

	class Playing final : public ::scene::inPlay::IScene
	{
	public:
		Playing( ) = delete;
		Playing( sf::RenderWindow& window, sf::Drawable& shapeOrSprite );
		~Playing( ) = default;

		void loadResources( ) override;
		void update( ::scene::inPlay::IScene** const nextScene ) override;
		void draw( ) override;
	private:
		// TODO: 로직은 따로 놓아볼까?
		Tetrimino spawn( );

		uint16_t mFrameCount;
		float mCellSize;
		sf::RenderWindow& mWindow;
		sf::RectangleShape& mBackgroundRect;
		Tetrimino mCurrentTetrimino;
		sf::RectangleShape mPlayerPanel;
		//sf::RectangleShape mOpponentPanel;
	};
}