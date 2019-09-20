#pragma hdrstop
#include "Socket.h"

int Socket::updateAcceptContext( Socket & listener )
{
	SOCKADDR_IN ignore1, ignore3;
	INT ignore2, ignore4;
	char ignore[ 1000 ];
	GetAcceptExSockaddrs( ignore, 0, 50, 50, (SOCKADDR**)&ignore1, &ignore2,
		(SOCKADDR**)&ignore3, &ignore4 );
	return setsockopt( mhSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
		(char*)&listener.mhSocket, sizeof( listener.mhSocket ) );
}

int Socket::receiveOverlapped( )
{
	WSABUF b;
	b.len = MAX_RCV_BUF_LEN;
	b.buf = mRcvBuffer;
	DWORD flag = 0;
	mCompletedWork = CompletedWork::RECEIVE;
	return WSARecv( mhSocket, &b, 1, NULL, &flag, &mOverlappedStruct, NULL );
}

int Socket::sendOverlapped( char * data, ULONG length )
{
	WSABUF b;
	b.len = length;
	b.buf = data;
	mCompletedWork = CompletedWork::SEND;
	return ::WSASend( mhSocket, &b, 1, NULL, 0, &mOverlappedStruct, NULL );
}
