#pragma once
#include "Playing.h"

class Room;

class Client
{
public:
	enum class State
	{
		UNVERIFIED,
		IN_LOBBY,
		IN_ROOM,
		READY,
		PLAYING,
		//INVITED,
	};

	Client( ) = delete;
	Client( const Socket::Type type, const Index index );
	void operator=( const Client& ) = delete;
	~Client( ) = default;

	bool update( std::unordered_map<HashedKey, Room>& roomS );

	Socket& socket( );
	void setSocket( const Socket& socket );
	State state( ) const;
	void setState( const State state );
	void holdTicket( const Ticket ticket );
	RoomID roomID( ) const;
	void setRoomID( const RoomID roomID );
	Playing& playing( );
private:
	Index mIndex;
	State mState;
	//�ñ�: �ʿ� ��������?
	Ticket mTicket;
	RoomID mRoomID;
	Request mLatestRequest;
	Playing mPlaying;
	//std::queue< ::model::Tetrimino > mNextTetriminoS;
	Socket mSocket;
};