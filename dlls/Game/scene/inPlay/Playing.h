#pragma once
#pragma hdrstop
#include <Lib/precompiled.h>
#include "IScene.h"

namespace scene::inPlay
{
	const uint8_t GRID_WIDTH = 10u;
	const uint8_t GRID_HEIGHT = 20u;
	// 90, 180, 270 and 360.
	const uint8_t ROTATION_NUM = 4u;
	const uint8_t BLOCKS_A_TETRIMINO = 4u;

	enum class TetriminoShape
	{
		I,
		J, L,
		N, S,
		T,
		O,

		NONE_MAX,
	};

	// BLOCKS_A_TETRIMINO x BLOCKS_A_TETRIMINO
	// 4 x 4
	using LocalSpace = uint16_t;
	struct Tetrimino
	{
		uint8_t rotationID;
		// On global coordinate.
		// Unit: Grid.
		sf::Vector2< int8_t > position;
		sf::Color color;
		LocalSpace rotations[ ROTATION_NUM ];
	};

	struct Cell
	{
		inline Cell( )
			: blocked( false ), color( sf::Color::Transparent ) { }
		bool blocked;
		sf::Color color;
	};

	class Playing final : public ::scene::inPlay::IScene
	{
	public:
		Playing( ) = delete;
		Playing( sf::RenderWindow& window, sf::Drawable& shapeOrSprite );
		~Playing( ) = default;

		void loadResources( ) override;
		void update( ::scene::inPlay::IScene** const nextScene, std::queue< sf::Event >& eventQueue ) override;
		void draw( ) override;
	private:
		// TODO: 로직은 따로 놓아볼까?
		Tetrimino spawn( );
		bool hasCollided( );

		bool mIsFallingDown;
		uint16_t mFrameCount;
		float mCellSize;
		sf::RenderWindow& mWindow;
		sf::RectangleShape& mBackgroundRect;
		Tetrimino mCurrentTetrimino;
		sf::RectangleShape mPlayerPanel;
		Cell mStage[ GRID_HEIGHT ][ GRID_WIDTH ];
		//sf::RectangleShape mOpponentPanel;
	};
}