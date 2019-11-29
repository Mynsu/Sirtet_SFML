#pragma once

#include <WinSock2.h>
#pragma comment (lib, "Ws2_32")
#include <stdint.h>
#include <SFML/System.hpp>
#include "Hash.h"

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
	TICKET,
	ORDER_IN_QUEUE,
	MY_NICKNAME,

	////
	// Request
	////
	REQ_NULL,
	REQ_CREATE_ROOM,
	REQ_START_GAME,
	REQ_GET_READY,
	REQ_LEAVE_ROOM,

	////
	// Inplay
	////
	MY_CURRENT_TETRIMINO,
	MY_STAGE,
};

using Tag = char[];
using Request = _Tag;

////
// Connection
////

// The queue server sends encrypted data attached by this tag to the both of them,
// the main server and ...
// ... the client having submitted the valid invitation.
// The client receives and sends to the main server as it is.
// With that the main server can see the connecting client is genuine or not.
constexpr Tag TAG_TICKET = { '0'+(int)_Tag::TICKET, ':', '\0' };

// The queue server sends the client's order attached by this tag.
constexpr Tag TAG_ORDER_IN_QUEUE = { '0'+(int)_Tag::ORDER_IN_QUEUE, ':', '\0' };

constexpr Tag TAG_MY_NICKNAME = { '0'+(int)_Tag::MY_NICKNAME, ':', '\0' };

////
// Request
////

constexpr char RESPONSE_AFFIRMATION = '1';
constexpr char RESPONSE_NEGATION = '0';

constexpr char TAG_REQ_CREATE_ROOM[ ] = { '0'+(int)_Tag::REQ_CREATE_ROOM, ':', '\0' };

constexpr char TAG_REQ_START_GAME[ ] = { '0'+(int)_Tag::REQ_START_GAME, ':', '\0' };

constexpr char TAG_REQ_GET_READY[ ] = { '0'+(int)_Tag::REQ_GET_READY, ':', '\0' };

constexpr char TAG_REQ_LEAVE_ROOM[ ] = { '0'+(int)_Tag::REQ_LEAVE_ROOM, ':', '\0' };

////
// Inplay
////

constexpr char TAG_MY_CURRENT_TETRIMINO[ ] = { '0'+(int)_Tag::MY_CURRENT_TETRIMINO, ':', '\0' };

constexpr char TAG_MY_STAGE[ ] = { '0'+(int)_Tag::MY_STAGE, ':', '\0' };