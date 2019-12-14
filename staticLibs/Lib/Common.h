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
	TICKET = 1,
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

// The queue server sends encrypted data attached by this tag to the both of them,
// the main server and ...
// ... the client having submitted the valid invitation.
// The client receives and sends to the main server as it is.
// With that the main server can see the connecting client is genuine or not.
constexpr Tag TAG_TICKET = { (char)_Tag::TICKET, ':', '\0' };
// The queue server sends the client's order attached by this tag.
constexpr Tag TAG_ORDER_IN_QUEUE = { '0'+(int)_Tag::ORDER_IN_QUEUE, ':', '\0' };
constexpr Tag TAG_MY_NICKNAME = { '0'+(int)_Tag::MY_NICKNAME, ':', '\0' };

////
// Request
////

constexpr char RESPONSE_AFFIRMATION = '1';
constexpr char RESPONSE_NEGATION = '0';

enum class Request
{
	NONE,
	CREATE_ROOM,
	START_GAME,
	GET_READY,
	LEAVE_ROOM,
};

constexpr Tag TAG_REQUEST = { '0'+(int)_Tag::REQUEST, ':', '\0' };
const uint8_t TAG_REQUEST_LEN = ::util::hash::Measure( TAG_REQUEST );
constexpr Tag TAGGED_REQ_CREATE_ROOM = { '0'+(int)_Tag::REQUEST, ':', '0'+(int)Request::CREATE_ROOM, '\0' };
constexpr Tag TAGGED_REQ_START_GAME = { '0'+(int)_Tag::REQUEST, ':', '0'+(int)Request::START_GAME, '\0' };
constexpr Tag TAGGED_REQ_GET_READY = { '0'+(int)_Tag::REQUEST, ':', '0'+(int)Request::GET_READY, '\0' };
constexpr Tag TAGGED_REQ_LEAVE_ROOM = { '0'+(int)_Tag::REQUEST, ':', '0'+(int)Request::LEAVE_ROOM, '\0' };

////
// Inplay
////

constexpr Tag TAG_MY_CURRENT_TETRIMINO = { '0'+(int)_Tag::MY_CURRENT_TETRIMINO, ':', '\0' };
constexpr Tag TAG_MY_NEXT_TETRIMINO = { '0'+(int)_Tag::MY_NEXT_TETRIMINO, ':', '\0' };
constexpr Tag TAG_MY_TEMPO_MS = { '0'+(int)_Tag::MY_TEMPO_MS, ':', '\0' };
constexpr Tag TAG_MY_STAGE = { '0'+(int)_Tag::MY_STAGE, ':', '\0' };
constexpr Tag TAG_MY_TETRIMINO_MOVE = { '0'+(int)_Tag::MY_TETRIMINO_MOVE, ':', '\0' };
const uint8_t TAG_MY_TETRIMINO_MOVE_LEN = ::util::hash::Measure( TAG_MY_TETRIMINO_MOVE );
constexpr Tag TAGGED_MY_TETRIMINO_MOVE_FALLDOWN = { '0'+(int)_Tag::MY_TETRIMINO_MOVE, ':',
												'0'+(int)::model::tetrimino::Move::FALL_DOWN, '\0' };
constexpr Tag TAGGED_MY_TETRIMINO_MOVE_DOWN = { '0'+(int)_Tag::MY_TETRIMINO_MOVE, ':',
											'0'+(int)::model::tetrimino::Move::DOWN, '\0' };
constexpr Tag TAGGED_MY_TETRIMINO_MOVE_LEFT = { '0'+(int)_Tag::MY_TETRIMINO_MOVE, ':',
											'0'+(int)::model::tetrimino::Move::LEFT, '\0' };
constexpr Tag TAGGED_MY_TETRIMINO_MOVE_RIGHT = { '0'+(int)_Tag::MY_TETRIMINO_MOVE, ':',
											'0'+(int)::model::tetrimino::Move::RIGHT, '\0' };
constexpr Tag TAGGED_MY_TETRIMINO_MOVE_ROTATE = { '0'+(int)_Tag::MY_TETRIMINO_MOVE, ':',
											'0'+(int)::model::tetrimino::Move::ROTATE, '\0' };
constexpr Tag TAG_MY_TETRIMINO_COLLIDED_IN_CLIENT = { '1', '0'+(int)_Tag::MY_TETRIMINO_COLLIDED_IN_CLIENT%10, ':', '\0' };
constexpr Tag TAG_MY_LINES_CLEARED = { '1', '0'+(int)_Tag::MY_LINES_CLEARED%10, ':', '\0' };
constexpr Tag TAG_MY_GAME_OVER = { '1', '0'+(int)_Tag::MY_GAME_OVER%10, ':', '\0' };