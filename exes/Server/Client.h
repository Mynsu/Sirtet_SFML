#pragma once

class Room;

class Client
{
public:
	enum class State
	{
		AS_QUEUE_SERVER = -1,
		UNVERIFIED,
		IN_LOBBY,
		WAITING_IN_ROOM,
		PLAYING_IN_ROOM,
	};

	Client( const Socket::Type type, const ClientIndex index );
	// !IMPORTANT: DO NOT USE.  Declared because std::vector requires.
	Client( const Client& )
		: mIndex( -1 ), mSocket( Socket::Type::TCP )
	{
#ifdef _DEBUG
		__debugbreak( );
#endif
	}
	void operator=( const Client& ) = delete;
	virtual ~Client( ) = default;

	std::vector<ClientIndex> work( const IOType completedIOType,
								std::vector<Client>& clientS,
								std::vector<ClientIndex>& lobby,
								std::unordered_map<HashedKey, Room>& roomS );
	Client::State state( ) const;
	void setState( const Client::State state );
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
	RoomID mRoomID;
	HashedKey mNicknameHashed_;
	std::string mNickname;
	Socket mSocket;
};