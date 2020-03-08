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

	// NOTE: std::array<Client,N>를 초기화하기 위해 기본 인자를 두었습니다.
	// std::array를 초기화하는데 필요한 기본 생성자를 이 생성자로 대체할 수 있었습니다.
	// 소켓은 생성 비용이 커, 복사를 하는 std::fill(...)이나 std::generate(...)를 쓰지 않았습니다.
	// !IMPORTANT: 엄연히 기본 생성자는 없습니다.
	Client( const Socket::Type type = Socket::Type::TCP, const ClientIndex index = -1 );
	Client( const Client& ) = delete;
	void operator=( const Client& ) = delete;
	virtual ~Client( ) = default;

	std::vector<ClientIndex> work( const IOType completedIOType,
								std::array<Client, CLIENT_CAPACITY>& clients,
								std::vector<ClientIndex>& lobby,
								std::unordered_map<HashedKey, Room>& rooms );
	void setIndex( const ClientIndex index )
	{
		mIndex = index;
	}
	Client::State state( ) const
	{
		return mState;
	}
	void setState( const Client::State state )
	{
		mState = state;
	}
	RoomID roomID( ) const
	{
		return mRoomID;
	}
	void setRoomID( const RoomID roomID )
	{
		mRoomID = roomID;
	}
	const std::string& nickname( ) const
	{
		return mNickname;
	}
	HashedKey nicknameHashed( ) const
	{
		return mNicknameHashed_;
	}
	Clock::time_point timeStamp( ) const
	{
		return mTimeStamp[(int)TimeStampIndex::GENERAL];
	}
	void resetTimeStamp( )
	{
		mTimeStamp[(int)TimeStampIndex::GENERAL] = Clock::now();
	}
	void setNickname( std::string& nickname )
	{
		mNickname.clear( );
		mNickname = nickname;
		mNicknameHashed_ = ::util::hash::Digest2(nickname);
	}
	Socket& socket( )
	{
		return mSocket;
	}
	void reset( const bool isSocketReusable = true );
private:
	enum class TimeStampIndex
	{
		GENERAL,
		TETRIMINO_MOVED,
		TETRIMINO_LANDED,
		NONE_MAX,
	};
	ClientIndex mIndex;
	RoomID mRoomID;
	Client::State mState;
	HashedKey mNicknameHashed_;
	Clock::time_point mTimeStamp[(int)TimeStampIndex::NONE_MAX];
	std::string mNickname;
	Socket mSocket;
};