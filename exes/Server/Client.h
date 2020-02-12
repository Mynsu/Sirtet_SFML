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
	Clock::time_point timeStamp() const
	{
		return mTimeStamp[(int)TimeStampIndex::GENERAL];
	}
	void resetTimeStamp()
	{
		mTimeStamp[(int)TimeStampIndex::GENERAL] = Clock::now();
	}
	void setNickname( std::string& nickname );
	Socket& socket( );
	void reset( const bool isSocketReusable = true );
private:
	enum class TimeStampIndex
	{
		GENERAL,
		TETRIMINO_MOVED,
		TETRIMINO_LANDED,
		NONE_MAX,
	};
	const ClientIndex mIndex;
	RoomID mRoomID;
	Client::State mState;
	HashedKey mNicknameHashed_;
	Clock::time_point mTimeStamp[(int)TimeStampIndex::NONE_MAX];
	std::string mNickname;
	Socket mSocket;
};