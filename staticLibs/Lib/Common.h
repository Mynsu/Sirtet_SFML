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
			///::model::tetrimino::Rotation rotationID;
		};

		const uint8_t BLOCKS_A_TETRIMINO = 4u;
		const uint8_t LOCAL_SPACE_SIZE = BLOCKS_A_TETRIMINO*BLOCKS_A_TETRIMINO;

		using LocalSpace = uint16_t;
	}
}

using Tag = char[];

// The queue server sends encrypted data attached by this tag to the both of them,
// the main server and ...
// ... the client having submitted the valid invitation.
// The client receives and sends to the main server as it is.
// With that the main server can see the connecting client is genuine or not.
constexpr Tag TAG_TICKET = "t:";
constexpr uint8_t TAG_TICKET_LEN = ::util::hash::Measure( TAG_TICKET );

// The queue server sends the client's order attached by this tag.
constexpr Tag TAG_ORDER_IN_QUEUE = "qL:";
constexpr uint8_t TAG_ORDER_IN_QUEUE_LEN = ::util::hash::Measure( TAG_ORDER_IN_QUEUE );

constexpr Tag TAG_NICKNAME = "nck:";
constexpr uint8_t TAG_NICKNAME_LEN = ::util::hash::Measure( TAG_NICKNAME );

//constexpr Tag TAG_NOTIFY_JOINING = "nJ:";
//constexpr uint8_t TAG_NOTIFY_JOINING_LEN = ::util::hash::Measure( TAG_NOTIFY_JOINING );
//
//constexpr Tag TAG_OLDERS = "old:";
//constexpr uint8_t TAG_OLDERS_LEN = ::util::hash::Measure( TAG_OLDERS );

enum class Request
{
	CREATE_ROOM,
	START_GAME,
	GET_READY,
	INVITE,
};

constexpr char RESPONSE_AFFIRMATION = '1';
constexpr char RESPONSE_NEGATION = '0';

constexpr char _TAG_CREATE_ROOM = '0' + (int)Request::CREATE_ROOM;
constexpr char TAG_CREATE_ROOM[] = { _TAG_CREATE_ROOM, ':', '\0' };

constexpr char _TAG_START_GAME = '0' + (int)Request::START_GAME;
constexpr char TAG_START_GAME[ ] = { _TAG_START_GAME, ':', '\0' };

constexpr char _TAG_GET_READY = '0' + (int)Request::GET_READY;
constexpr char TAG_GET_READY[ ] = { _TAG_GET_READY, ':', '\0' };

//constexpr char _TAG_INVITE = '0' + (int)Request::INVITE;
//constexpr char TAG_INVITE[] = { _TAG_INVITE, ':', '\0' };

////
// inPlay
////

constexpr char TAG_CURRENT_TETRIMINO[ ] = "curT:";