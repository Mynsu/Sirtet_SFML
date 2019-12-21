#pragma once

class Room;

class Client
{
public:
	enum class State
	{
		UNVERIFIED,
		IN_LOBBY,
		WAITING_IN_ROOM,
		PLAYING_IN_ROOM,
	};

	Client( ) = delete;
	Client( const Socket::Type type, const ClientIndex index );
	Client( const Client& ) = default;
	void operator=( const Client& ) = delete;
	~Client( ) = default;

	bool complete( std::vector<Client>& clientS,
				  std::unordered_set<ClientIndex>& lobby,
				  std::unordered_map<HashedKey, Room>& roomS );
	Client::State state( ) const;
	void setState( const Client::State state );
	void holdTicket( const Ticket ticket );
	RoomID roomID( ) const;
	void setRoomID( const RoomID roomID );
	const std::string& nickname( ) const;
	void setNickname( std::string& nickname );
	Socket& socket( );
	void setSocket( const Socket::Type type, const Socket::CompletedWork completedWork );
	void reset( );
private:
	const ClientIndex mIndex;
	Client::State mState;
	//궁금: 필요 있으려나?
	Ticket mTicket;
	RoomID mRoomID;
	Request mRecentRequest;
	std::string mNickname;
	Socket mSocket;
};