#pragma once
#include <cstdint>

namespace model
{
	namespace stage
	{
		const uint8_t GRID_WIDTH = 10u;
		const uint8_t GRID_HEIGHT = 20u;
	}

	namespace tetrimino
	{
		enum class Type
		{
			I,
			J, L,
			N, S,
			T,
			O,

			NONE_MAX,
		};

		const uint8_t BLOCKS_A_TETRIMINO = 4u;
	}
}