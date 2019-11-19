#include "pch.h"
#include "Client.h"

Client::Client( const Socket::Type type )
	: mState( State::UNVERIFIED ), mTicket( 0u ), mRoomID( 0u ), mLatestRequest( (Request)-1 ),
	mSocket( type )
{
}

Socket& Client::socket( )
{
	return mSocket;
}

void Client::setSocket( const Socket& socket )
{
	mSocket.close( );
	mSocket = socket;
}

Client::State Client::state( ) const
{
	return mState;
}

void Client::setState( const State state )
{
	mState = state;
	switch ( state )
	{
		case State::READY:
			mPlaying.spawnTetrimino( );
			break;
	//	case State::PLAYING:
	//		break;
		default:
			break;
	}
}

void Client::holdTicket( const Ticket ticket )
{
	mTicket = ticket;
}

Ticket Client::ticket( ) const
{
	return mTicket;
}

RoomID Client::roomID( ) const
{
	return mRoomID;
}

void Client::setRoomID( const RoomID roomID )
{
	mRoomID = roomID;
}

std::string Client::currentTetriminoInfo( )
{
	return mPlaying.currentTetriminoInfo( );
}

Request Client::latestRequest( ) const
{
	return mLatestRequest;
}

void Client::setLatestRequest( const Request request )
{
	mLatestRequest = request;
}
