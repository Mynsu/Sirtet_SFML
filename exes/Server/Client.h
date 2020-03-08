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

	// NOTE: std::array<Client,N>�� �ʱ�ȭ�ϱ� ���� �⺻ ���ڸ� �ξ����ϴ�.
	// std::array�� �ʱ�ȭ�ϴµ� �ʿ��� �⺻ �����ڸ� �� �����ڷ� ��ü�� �� �־����ϴ�.
	// ������ ���� ����� Ŀ, ���縦 �ϴ� std::fill(...)�̳� std::generate(...)�� ���� �ʾҽ��ϴ�.
	// !IMPORTANT: ������ �⺻ �����ڴ� �����ϴ�.
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