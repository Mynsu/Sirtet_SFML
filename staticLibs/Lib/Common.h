#pragma once
#include <array>
#include <SFML/System.hpp>
#include <SFML/Graphics/Color.hpp>
#include "Hash.h"

namespace model
{
	namespace stage
	{
		const uint8_t GRID_WIDTH = 10u;
		const uint8_t GRID_HEIGHT = 20u;

		// A stage consists of cells as a tetrimino consists of blocks.
		// Strictly, cell isn't block and vice versa, but they match up each other.
		struct Cell
		{
			inline Cell( )
				: blocked( false ), color( sf::Color::Transparent )
			{
			}
			bool blocked;
			sf::Color color;
		};

		using Grid = std::array< std::array<Cell, ::model::stage::GRID_WIDTH>, ::model::stage::GRID_HEIGHT >;
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

			NONE_MAX,
		};

		enum class Move
		{
			FALL_DOWN,
			DOWN,
			LEFT,
			RIGHT,
			ROTATE,
			NONE_MAX,
		};

		const uint8_t BLOCKS_A_TETRIMINO = 4u;
		const uint8_t LOCAL_SPACE_SIZE = BLOCKS_A_TETRIMINO*BLOCKS_A_TETRIMINO;

		using LocalSpace = uint16_t;
	}
}

constexpr char TOKEN_SEPARATOR = ' ';
constexpr char TOKEN_SEPARATOR_2 = '_';

enum class _Tag
{
	////
	// Connection
	////
	POPULATION = 1,
	TICKET,
	ORDER_IN_QUEUE,
	MY_NICKNAME,

	////
	// Request
	////
	REQUEST,

	////
	// Inplay
	////
	MY_CURRENT_TETRIMINO,
	MY_NEXT_TETRIMINO,
	MY_TEMPO_MS,
	MY_STAGE,
	MY_TETRIMINO_MOVE,
	MY_TETRIMINO_COLLIDED_IN_CLIENT,
	MY_LINES_CLEARED,
	MY_GAME_OVER,
};

using Tag = char[];

////
// Connection
////

// 4 Bytes
constexpr Tag TAG_POPULATION = { (char)_Tag::POPULATION, ':', '\0' };
const uint8_t TAG_POPULATION_LEN = ::util::hash::Measure( TAG_POPULATION );
// The queue server sends encrypted data attached by this tag to the both of them,
// the main server and ...
// ... the client having submitted the valid invitation.
// The client receives and sends to the main server as it is.
// With that the main server can see the connecting client is genuine or not.
// 4 Bytes
constexpr Tag TAG_TICKET = { (char)_Tag::TICKET, ':', '\0' };
// The queue server sends the client's order attached by this tag.
// 4 Bytes
constexpr Tag TAG_ORDER_IN_QUEUE = { (char)_Tag::ORDER_IN_QUEUE, ':', '\0' };
// Resilient size
constexpr Tag TAG_MY_NICKNAME = { (char)_Tag::MY_NICKNAME, ':', '\0' };

////
// Request
////

enum class Request
{
	NONE,
	UPDATE_USER_LIST,
	CREATE_ROOM,
	START_GAME,
	GET_READY,
	LEAVE_ROOM,
};

// 1 Byte
constexpr Tag TAG_REQUEST = { (char)_Tag::REQUEST, ':', '\0' };
const uint8_t TAG_REQUEST_LEN = ::util::hash::Measure( TAG_REQUEST );
constexpr Tag TAGGED_REQ_USER_LIST = { (char)_Tag::REQUEST, ':', (char)Request::UPDATE_USER_LIST, '\0' };
constexpr Tag TAGGED_REQ_CREATE_ROOM = { (char)_Tag::REQUEST, ':', (char)Request::CREATE_ROOM, '\0' };
constexpr Tag TAGGED_REQ_START_GAME = { (char)_Tag::REQUEST, ':', (char)Request::START_GAME, '\0' };
constexpr Tag TAGGED_REQ_GET_READY = { (char)_Tag::REQUEST, ':', (char)Request::GET_READY, '\0' };
constexpr Tag TAGGED_REQ_LEAVE_ROOM = { (char)_Tag::REQUEST, ':', (char)Request::LEAVE_ROOM, '\0' };

////
// Inplay
////

// Type( 4 Bytes )
constexpr Tag TAG_MY_CURRENT_TETRIMINO = { (char)_Tag::MY_CURRENT_TETRIMINO, ':', '\0' };
constexpr Tag TAG_MY_NEXT_TETRIMINO = { (char)_Tag::MY_NEXT_TETRIMINO, ':', '\0' };
// 4 Bytes
constexpr Tag TAG_MY_TEMPO_MS = { (char)_Tag::MY_TEMPO_MS, ':', '\0' };
// Resilient size
constexpr Tag TAG_MY_STAGE = { (char)_Tag::MY_STAGE, ':', '\0' };
// 1 Byte
constexpr Tag TAG_MY_TETRIMINO_MOVE = { (char)_Tag::MY_TETRIMINO_MOVE, ':', '\0' };
const uint8_t TAG_MY_TETRIMINO_MOVE_LEN = ::util::hash::Measure( TAG_MY_TETRIMINO_MOVE );
// Zero Byte
constexpr Tag TAG_MY_TETRIMINO_COLLIDED_IN_CLIENT = { (char)_Tag::MY_TETRIMINO_COLLIDED_IN_CLIENT, ':', '\0' };
// 1 Bytes
constexpr Tag TAG_MY_LINES_CLEARED = { (char)_Tag::MY_LINES_CLEARED, ':', '\0' };
// Zero Byte
constexpr Tag TAG_MY_GAME_OVER = { (char)_Tag::MY_GAME_OVER, ':', '\0' };