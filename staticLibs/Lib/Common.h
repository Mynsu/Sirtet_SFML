#pragma once

#include <WinSock2.h>
#include <stdint.h>

#pragma comment (lib, "Ws2_32")

#include <SFML/System.hpp>

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

		enum class Rotation
		{
			A,
			B,
			C,
			D,

			NULL_MAX,
		};

		struct Info
		{
			sf::Vector2<int8_t> position;
			::model::tetrimino::Type type;
			///::model::tetrimino::Rotation rotationID;
		};

		const uint8_t BLOCKS_A_TETRIMINO = 4u;
	}
}