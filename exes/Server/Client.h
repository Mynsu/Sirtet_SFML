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
		AS_QUEUE_SERVER,
	};

	Client( ) = delete;
	Client( const Socket::Type type, const ClientIndex index );
	Client( const Client& ) = default;
	void operator=( const Client& ) = delete;
	~Client( ) = default;

	std::vector<ClientIndex> work( const IOType completedIOType,
								std::vector<Client>& clientS,
								std::vector<ClientIndex>& lobby,
								std::unordered_map<HashedKey, Room>& roomS );
	Client::State state( ) const;
	void setState( const Client::State state );
	void holdTicket( const Ticket ticket );
	RoomID roomID( ) const;
	void setRoomID( const RoomID roomID );
	const std::string& nickname( ) const;
	HashedKey nicknameHashed() const;
	void setNickname( std::string& nickname );
	Socket& socket( );
	void reset( const bool isSocketReusable = true );
private:
	const ClientIndex mIndex;
	Client::State mState;
	//궁금: 필요 있으려나?
	Ticket mTicket;
	RoomID mRoomID;
	HashedKey mNicknameHashed_;
	std::string mNickname;
	Socket mSocket;
};