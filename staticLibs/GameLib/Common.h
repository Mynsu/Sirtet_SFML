#pragma once
#include <array>
#include <chrono>
#include <SFML/Graphics/Color.hpp>
#include "../Lib/math/Vector.h"
#include "../Lib/Hash.h"

static const char* QUEUE_SERVER_IP_ADDRESS = "127.0.0.1";
static const char* MAIN_SERVER_IP_ADDRESS = "127.0.0.1";
const uint16_t QUEUE_SERVER_PORT = 10000;
const uint16_t MAIN_SERVER_PORT = 54321;
const uint16_t MAX_KEY_STRETCHING = 5000;
// Recommended to be renewed periodically for security.
const uint32_t VERSION = 200211;
const uint32_t SALT = VERSION;

struct MouseEvent
{
	std::chrono::high_resolution_clock::time_point latestClickTime;
	math::Vector<2> clickPosition;
};

namespace model
{
	namespace stage
	{
		const uint8_t GRID_WIDTH = 10;
		const uint8_t GRID_HEIGHT = 20;

		// A stage consists of CELLs as a tetrimino consists of BLOCKs.
		// Strictly, cell isn't block and vice versa, but they match up each other.
		struct Cell
		{
			Cell( )
				: blocked( false ), color( sf::Color::Transparent )
			{
			}
			bool blocked;
			sf::Color color;
		};

		using Grid = std::array<std::array<Cell, ::model::stage::GRID_WIDTH>, ::model::stage::GRID_HEIGHT>;
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
			// Clockwise
			A, // PI/2
			B, // PI
			C, // 
			D, // 2*PI

			NONE_MAX,
		};

		enum class Move
		{
			HARD_DROP,
			DOWN, // Soft drop
			LEFT,
			RIGHT,
			ROTATE,
			NONE_MAX,
		};

		const uint8_t BLOCKS_A_TETRIMINO = 4;
		const uint8_t LOCAL_SPACE_SIZE = BLOCKS_A_TETRIMINO*BLOCKS_A_TETRIMINO;

		using LocalSpace = uint16_t;
	}
}

enum class _Tag
{
	////
	// Connection
	////
	// !IMPORTANT: 0 equals '\0' that might cause an error.
	INVITATION = 1,
	NUM_OF_CONNECTIONS,
	TICKET,
	ORDER_IN_QUEUE,
	MY_NICKNAME,

	////
	// Request
	// 세부 태그는 아래 참조.
	////
	REQUEST,

	////
	// Notification
	// 세부 태그는 아래 참조.
	////
	NOTIFICATION,

	////
	// Inplay
	////
	MY_TEMPO_MS,
	MY_TETRIMINO_MOVE,
	MY_TETRIMINO_LANDED_ON_CLIENT,

	CURRENT_TETRIMINOS,
	CURRENT_TETRIMINOS_MOVE,
	CURRENT_TETRIMINOS_LAND,
	NEXT_TETRIMINOS,
	STAGES,
	NUMS_OF_LINES_CLEARED,
	GAMES_OVER,
	ALL_OVER,
};

using Tag = char[];

////
// Connection
////

// 초대장invitation으로 대기열 서버는 클라이언트가 변조되지 않았는지, 버전이 서버들과 맞는지 확인할 수 있습니다.
// Attached to uint32_t.
constexpr Tag TAG_INVITATION = { (char)_Tag::INVITATION, ':', '\0' };
// Attached to uint16_t.
constexpr Tag TAG_NUM_OF_CONNECTIONS = { (char)_Tag::NUM_OF_CONNECTIONS, ':', '\0' };
// 대기열 서버를 통해 정상적으로 메인 서버에 접속한 클라이언트는 메인 서버에 티켓을 제출합니다.
// Attached to HashedKey.
constexpr Tag TAG_TICKET = { (char)_Tag::TICKET, ':', '\0' };
// 대기열 서버가 클라이언트에게 대기열 번호를 알려줍니다.
// Attached to uint16_t.
constexpr Tag TAG_ORDER_IN_QUEUE = { (char)_Tag::ORDER_IN_QUEUE, ':', '\0' };
// Attached to (variable-length string).
constexpr Tag TAG_MY_NICKNAME = { (char)_Tag::MY_NICKNAME, ':', '\0' };

////
// Request
// 클라이언트가 서버에 무언가를 요청할 때 붙입니다.
// 때로 서버가 클라이언트에 요청에 대한 응답을 보낼 때도 붙입니다.
////

enum class Request
{
	// !IMPORTANT: 0 equals '\0' that might cause an error.
	UPDATE_USER_LIST = 1,
	CREATE_ROOM,
	START_GAME,
	GET_READY,
	LEAVE_ROOM,
	JOIN_ROOM,
};

// Attached to uint8_t(for enum type Request).
constexpr Tag TAG_REQUEST = { (char)_Tag::REQUEST, ':', '\0' };
constexpr uint8_t TAG_REQUEST_LEN = ::util::hash::Measure(TAG_REQUEST);
// Attached to nothing.
constexpr Tag TAGGED_REQ_USER_LIST = { (char)_Tag::REQUEST, ':', (char)Request::UPDATE_USER_LIST, '\0' };
// Attached to nothing.
constexpr Tag TAGGED_REQ_CREATE_ROOM = { (char)_Tag::REQUEST, ':', (char)Request::CREATE_ROOM, '\0' };
// Attached to nothing.
constexpr Tag TAGGED_REQ_START_GAME = { (char)_Tag::REQUEST, ':', (char)Request::START_GAME, '\0' };
// Attached to nothing.
constexpr Tag TAGGED_REQ_GET_READY = { (char)_Tag::REQUEST, ':', (char)Request::GET_READY, '\0' };
// Attached to nothing.
constexpr Tag TAGGED_REQ_LEAVE_ROOM = { (char)_Tag::REQUEST, ':', (char)Request::LEAVE_ROOM, '\0' };
// When a client requests for the server, this's attached to (variable-length string for a nickname).
// When the server responds to the client, this's attached to uint8_t(for enum type ResultJoiningRoom).
constexpr Tag TAGGED_REQ_JOIN_ROOM = { (char)_Tag::REQUEST, ':', (char)Request::JOIN_ROOM, '\0' };
constexpr uint8_t TAGGED_REQ_JOIN_ROOM_LEN = ::util::hash::Measure(TAGGED_REQ_JOIN_ROOM);
enum class ResultJoiningRoom
{
	FAILED_BY_SERVER_ERROR = -1,
	// !IMPORTANT: 0 equals '\0' that might cause an error.
	SUCCCEDED = 1,
	FAILED_DUE_TO_SELF_TARGET,
	FAILED_DUE_TO_TARGET_NOT_CONNECTING,
	FAILED_BY_FULL_ROOM,
	NONE,
};

////
// Notification
// 서버가 클라이언트들에게 무언가를 알릴 때 붙입니다.
////

enum class Notification
{
	// !IMPORTANT: 0 equals '\0' that might cause an error.
	UPDATE_USER_LIST = 1,
	HOST_CHANGED,
};

// Attached to uint16_t(for the total size) and
// both of uint8_t(for the size of each nickname) and variable-length string(for each nickname) repeats.
constexpr Tag TAGGED_NOTI_UPDATE_USER_LIST = { (char)_Tag::NOTIFICATION, ':', (char)Notification::UPDATE_USER_LIST, '\0' };
// Attached to HashedKey(for a hashed nickname).
constexpr Tag TAGGED_NOTI_HOST_CHANGED = { (char)_Tag::NOTIFICATION, ':', (char)Notification::HOST_CHANGED, '\0' };

////
// Inplay
////

// Attached to uint16_t(for milliseconds).
constexpr Tag TAG_MY_TEMPO_MS = { (char)_Tag::MY_TEMPO_MS, ':', '\0' };
// Attached to uint8_t(for enum type ::model::tetrimino::Move).
constexpr Tag TAG_MY_TETRIMINO_MOVE = { (char)_Tag::MY_TETRIMINO_MOVE, ':', '\0' };
const uint8_t TAG_MY_TETRIMINO_MOVE_LEN = ::util::hash::Measure(TAG_MY_TETRIMINO_MOVE);
// Attached to nothing.
constexpr Tag TAG_MY_TETRIMINO_LANDED_ON_CLIENT = { (char)_Tag::MY_TETRIMINO_LANDED_ON_CLIENT, ':', '\0' };
// Attached to uint16_t(for the total size) and
// both of HashedKey(for the hashed nickname of each tetrimino owner) and uint8_t(for enum type ::model::tetrimino::Type) repeats.
constexpr Tag TAG_CURRENT_TETRIMINOS = { (char)_Tag::CURRENT_TETRIMINOS, ':', '\0' };
// Attached to uint16_t(for the total size) and,
// all of HashedKey(for the hashed nickname of each tetrimino owner),
// uint8_t(for enum type ::model::tetrimino::Rotation) and
// sf::Vector2<int8_t>(for its destination) repeats.
constexpr Tag TAG_CURRENT_TETRIMINOS_MOVE = { (char)_Tag::CURRENT_TETRIMINOS_MOVE, ':', '\0' };
// Attached to uint16_t(for the total size) and,
// HashedKeys(for the hashed nickname of each tetrimino owner) repeats.
constexpr Tag TAG_CURRENT_TETRIMINOS_LAND = { (char)_Tag::CURRENT_TETRIMINOS_LAND, ':', '\0' };
// Attached to uint16_t(for the total size) and
// both of HashedKey(for the hashed nickname of each tetrimino owner) and uint8_t(for enum type ::model::tetrimino::Type) repeats.
constexpr Tag TAG_NEXT_TETRIMINOS = { (char)_Tag::NEXT_TETRIMINOS, ':', '\0' };
// Attached to uint16_t(for the total size) and
// both of HashedKey(for the hashed nickname of each player) and type ::model::Stage::Grid repeats.
constexpr Tag TAG_STAGES = { (char)_Tag::STAGES, ':', '\0' };
// Attached to uint16_t(for the total size) and
// both of HashedKey(for the hashed nickname of each player) and uint8_t repeats.
constexpr Tag TAG_NUMS_OF_LINES_CLEARED = { (char)_Tag::NUMS_OF_LINES_CLEARED, ':', '\0' };
// Attached to uint16_t(for the total size) and,
// HashedKeys(for the hashed nickname of each player) repeats.
constexpr Tag TAG_GAMES_OVER = { (char)_Tag::GAMES_OVER, ':', '\0' };
// Attached to nothing.
constexpr Tag TAG_ALL_OVER = { (char)_Tag::ALL_OVER, ':', '\0' };