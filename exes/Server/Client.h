#pragma once
#include "Playing.h"

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
	Client( const Socket::Type type );
	///Client( const Client& )	= delete;
	///void operator=( const Client& ) = delete;
	~Client( ) = default;

	Socket& socket( );
	void setSocket( const Socket& socket );
	State state( ) const;
	void setState( const State state );
	void holdTicket( const Ticket ticket );
	Ticket ticket( ) const;
	RoomID roomID( ) const;
	void setRoomID( const RoomID roomID );
	std::string currentTetriminoInfo( );
	Request latestRequest( ) const;
	void setLatestRequest( const Request request );
private:
	State mState;
	//궁금: 필요 있으려나?
	Ticket mTicket;
	RoomID mRoomID;
	Request mLatestRequest;
	Playing mPlaying;
	//std::queue< ::model::Tetrimino > mNextTetriminoS;
	Socket mSocket;
};