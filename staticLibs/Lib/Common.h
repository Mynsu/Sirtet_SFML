#pragma once
#include <array>
#include <SFML/System.hpp>
#include <SFML/Graphics/Color.hpp>
#include "Hash.h"

static const char* QUEUE_SERVER_IP_ADDRESS = "192.168.219.102";
static const char* MAIN_SERVER_IP_ADDRESS = "192.168.219.102";
const uint16_t QUEUE_SERVER_PORT = 10000u;
const uint16_t MAIN_SERVER_PORT = 54321u;

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

enum class _Tag
{
	////
	// Connection
	////
	INVITATION = 1,
	POPULATION,
	TICKET,
	ORDER_IN_QUEUE,
	MY_NICKNAME,

	////
	// Request
	////
	REQUEST,

	////
	// Notification
	////
	NOTIFICATION,

	////
	// Inplay
	////
	//TODO: ÇÑ Ãþ ´õ µÑ±î?
	MY_NEW_CURRENT_TETRIMINO,
	MY_NEXT_TETRIMINO,
	MY_TEMPO_MS,
	MY_STAGE,
	MY_TETRIMINO_MOVE,
	MY_TETRIMINO_COLLIDED_IN_CLIENT,
	MY_NUM_OF_LINES_CLEARED,
	MY_GAME_OVER,

	NEW_CURRENT_TETRIMINOS,
	CURRENT_TETRIMINOS_MOVE,
	///NEXT_TETRIMINOS,
	STAGES,
	NUMS_OF_LINES_CLEARED,
	GAMES_OVER,
	ALL_OVER,
};

using Tag = char[];

////
// Connection
////

// Attached to HashedKey.
constexpr Tag TAG_INVITATION = { (char)_Tag::INVITATION, ':', '\0' };
// Attached to uint32_t.
constexpr Tag TAG_POPULATION = { (char)_Tag::POPULATION, ':', '\0' };
// The queue server sends encrypted data attached by this tag to the both of them,
// the main server and ...
// ... the client having submitted the valid invitation.
// The client receives and sends to the main server as it is.
// With that the main server can see the connecting client is genuine or not.
// Attached to HashedKey.
constexpr Tag TAG_TICKET = { (char)_Tag::TICKET, ':', '\0' };
// The queue server sends the client's order attached by this tag.
// Attached to uint32_t.
constexpr Tag TAG_ORDER_IN_QUEUE = { (char)_Tag::ORDER_IN_QUEUE, ':', '\0' };
// Attached to std::string
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
	JOIN_ROOM,
};

// Attached to char(for Request).
constexpr Tag TAG_REQUEST = { (char)_Tag::REQUEST, ':', '\0' };
const uint8_t TAG_REQUEST_LEN = ::util::hash::Measure( TAG_REQUEST );
// Attached to nothing.
constexpr Tag TAGGED_REQ_USER_LIST_IN_LOBBY = { (char)_Tag::REQUEST, ':', (char)Request::UPDATE_USER_LIST, '\0' };
// Attached to nothing.
constexpr Tag TAGGED_REQ_CREATE_ROOM = { (char)_Tag::REQUEST, ':', (char)Request::CREATE_ROOM, '\0' };
// Attached to nothing.
constexpr Tag TAGGED_REQ_START_GAME = { (char)_Tag::REQUEST, ':', (char)Request::START_GAME, '\0' };
// Attached to nothing.
constexpr Tag TAGGED_REQ_GET_READY = { (char)_Tag::REQUEST, ':', (char)Request::GET_READY, '\0' };
// Attached to nothing.
constexpr Tag TAGGED_REQ_LEAVE_ROOM = { (char)_Tag::REQUEST, ':', (char)Request::LEAVE_ROOM, '\0' };
// When a client requests for the server, this's attached to std::string(for a nickname).
// When the server responds to the client, this's attached to uint8_t(for ResultJoiningRoom).
constexpr Tag TAGGED_REQ_JOIN_ROOM = { (char)_Tag::REQUEST, ':', (char)Request::JOIN_ROOM, '\0' };
constexpr uint8_t TAGGED_REQ_JOIN_ROOM_LEN = ::util::hash::Measure(TAGGED_REQ_JOIN_ROOM);
enum class ResultJoiningRoom
{
	FAILED_BY_SERVER_ERROR = -1,
	///
	FAILED_BY_FULL_ROOM,
	SUCCCEDED,
	FAILED_DUE_TO_TARGET_NOT_CONNECTING,
	FAILED_DUE_TO_SELF_TARGET,
	NONE,
};

////
// Notification
////
enum class Notification
{
	// !IMPORTANT: 0 equals '\0'
	SOMEONE_VISITED = 1,
	SOMEONE_LEFT,
	UPDATE_USER_LIST,
};

// Attached to std::string(for a nickname).
constexpr Tag TAGGED_NOTI_SOMEONE_VISITED = { (char)_Tag::NOTIFICATION, ':', (char)Notification::SOMEONE_VISITED, '\0' };
// Attached to std::string(for a nickname).
constexpr Tag TAGGED_NOTI_SOMEONE_LEFT = { (char)_Tag::NOTIFICATION, ':', (char)Notification::SOMEONE_LEFT, '\0' };
// Attached to uint32_t(for the total size) and repeated pairs of <uint8_t,std::string>.
constexpr Tag TAGGED_NOTI_UPDATE_USER_LIST = { (char)_Tag::NOTIFICATION, ':', (char)Notification::UPDATE_USER_LIST, '\0' };

////
// Inplay
////

// Attached to uint8_t(for ::model::tetrimino::Type).
constexpr Tag TAG_MY_NEW_CURRENT_TETRIMINO = { (char)_Tag::MY_NEW_CURRENT_TETRIMINO, ':', '\0' };
// Attached to uint8_t(for ::model::tetrimino::Type).
constexpr Tag TAG_MY_NEXT_TETRIMINO = { (char)_Tag::MY_NEXT_TETRIMINO, ':', '\0' };
// Attached to uint32_t(for milliseconds).
constexpr Tag TAG_MY_TEMPO_MS = { (char)_Tag::MY_TEMPO_MS, ':', '\0' };
// Attached to Grid.
constexpr Tag TAG_MY_STAGE = { (char)_Tag::MY_STAGE, ':', '\0' };
// Attached to uint8_t(for ::model::tetrimino::Move).
constexpr Tag TAG_MY_TETRIMINO_MOVE = { (char)_Tag::MY_TETRIMINO_MOVE, ':', '\0' };
const uint8_t TAG_MY_TETRIMINO_MOVE_LEN = ::util::hash::Measure( TAG_MY_TETRIMINO_MOVE );
// Attached to nothing.
constexpr Tag TAG_MY_TETRIMINO_COLLIDED_ON_CLIENT = { (char)_Tag::MY_TETRIMINO_COLLIDED_IN_CLIENT, ':', '\0' };
// Attached to uint8_t(for the number of cleared lines).
constexpr Tag TAG_MY_NUM_OF_LINES_CLEARED = { (char)_Tag::MY_NUM_OF_LINES_CLEARED, ':', '\0' };
// Attached to nothing.
constexpr Tag TAG_MY_GAME_OVER = { (char)_Tag::MY_GAME_OVER, ':', '\0' };
// Attached to uint32_t(for the total size) and repeated pairs of <HashedKey,uint8_t>.

constexpr Tag TAG_NEW_CURRENT_TETRIMINOS = { (char)_Tag::NEW_CURRENT_TETRIMINOS, ':', '\0' };
// Attached to uint32_t(for the total size) and repeated tuples of <HashedKey,uint8_t,sf::Vector2<int8_t>>.
constexpr Tag TAG_CURRENT_TETRIMINOS_MOVE = { (char)_Tag::CURRENT_TETRIMINOS_MOVE, ':', '\0' };
/// Attached to uint8_t(for ::model::tetrimino::Type).
///constexpr Tag TAG_NEXT_TETRIMINOS = { (char)_Tag::NEXT_TETRIMINOS, ':', '\0' };
// Attached to std::string(for Grid).
constexpr Tag TAG_STAGES = { (char)_Tag::STAGES, ':', '\0' };
// Attached to uint8_t.
constexpr Tag TAG_NUMS_OF_LINES_CLEARED = { (char)_Tag::NUMS_OF_LINES_CLEARED, ':', '\0' };
// Attached to nothing.
constexpr Tag TAG_GAMES_OVER = { (char)_Tag::GAMES_OVER, ':', '\0' };
// Attached to nothing.
constexpr Tag TAG_ALL_OVER = { (char)_Tag::ALL_OVER, ':', '\0' };