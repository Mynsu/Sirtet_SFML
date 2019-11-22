#pragma once

class Room;

using ClientIndex = uint32_t;
using Ticket = HashedKey;

class Client
{
public:
	enum class State
	{
		UNVERIFIED,
		IN_LOBBY,
		IN_ROOM,
	};

	Client( ) = delete;
	Client( const Socket::Type type, const ClientIndex index );
	Client( const Client& ) = default;
	void operator=( const Client& ) = delete;
	~Client( ) = default;

	// NOTE: Called only in Server.cpp.  Not called in Room.cpp.
	bool complete( std::unordered_map<HashedKey, Room>& roomS );
	void setState( const Client::State state );
	Client::State state( ) const;
	void holdTicket( const Ticket ticket );
	RoomID roomID( ) const;
	void setRoomID( const RoomID roomID );
	Socket& socket( );
	void setSocket( const Socket& socket );
private:
	ClientIndex mIndex;
	Client::State mState;
	//궁금: 필요 있으려나?
	Ticket mTicket;
	RoomID mRoomID;
	Request mRecentRequest;
	Socket mSocket;
};