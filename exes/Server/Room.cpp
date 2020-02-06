#include "pch.h"
#include "Room.h"
#include "Client.h"

const uint32_t UPDATE_USER_LIST_INTERVAL_MS = 1000;
const uint32_t COUNTDOWN_MS = 3000;
const int32_t TEMPO_DIFF_MS = -20;

Room::Room( const ClientIndex hostIndex )
	: mHasTempoChanged_( false ), mHasHostChanged_( false ),
	mHostIndex( hostIndex ), mState( State::WAITING )
{
	mParticipants.reserve( PARTICIPANT_CAPACITY );
	mCandidateParticipants.emplace_back( hostIndex );
	const Clock::time_point init = Clock::now();
	for ( auto& it : mAlarms )
	{
		it = init;
	}
}

void Room::start( )
{
	mState = State::ON_START;
}

int Room::leave( const ClientIndex index )
{
	bool isThereSomeoneRemains = true;
	bool wasCandidateParticipant = false;
	for ( auto it = mCandidateParticipants.begin(); mCandidateParticipants.end() != it; ++it )
	{
		if ( index == *it )
		{
			mCandidateParticipants.erase( it );
			wasCandidateParticipant = true;
			break;
		}
	}
	if ( false == wasCandidateParticipant )
	{
#ifdef _DEBUG
		if ( 1 != mParticipants.erase(index) )
		{
			__debugbreak( );
		}
#else
		mParticipants.erase( index );
#endif
	}

	if ( const uint8_t pop = (uint8_t)(mCandidateParticipants.size()+mParticipants.size());
		0 == pop )
	{
		isThereSomeoneRemains = false;
	}
	else if ( index == mHostIndex )
	{
		if ( const auto it = mParticipants.cbegin();
			mParticipants.cend() != it )
		{
			mHostIndex = it->first;
		}
		else
		{
			const auto it2 = mCandidateParticipants.cbegin();
			mHostIndex = *it2;
		}
		mHasHostChanged_ = true;
	}
	return (true==isThereSomeoneRemains)? 1: 0;
}

void Room::perceive( const ClientIndex index, const ::model::tetrimino::Move move )
{
	if ( auto it = mParticipants.find(index); mParticipants.end() != it )
	{
		it->second.perceive( move );
	}
	// Exception
	else
	{
		std::cerr << "Client " << index << " knows the incorrect room ID.\n";
	}
}

void Room::perceive( const ClientIndex index, const bool hasTetriminoCollidedInClient )
{
	if ( auto it = mParticipants.find(index); mParticipants.end() != it )
	{
		it->second.perceive( );
	}
	// Exception
	else
	{
		std::cerr << "Client " << index << " knows the incorrect room ID.\n";
	}
}

std::vector<ClientIndex> Room::update( std::vector<Client>& clients )
{
	std::vector<ClientIndex> failedIndices;
	switch ( mState )
	{
		case Room::State::WAITING:
			break;
		case Room::State::ON_START:
			for ( const ClientIndex idx : mCandidateParticipants )
			{
				mParticipants.emplace( idx, Playing() );
				clients[idx].setState( Client::State::PLAYING_IN_ROOM );
			}
			mCandidateParticipants.clear( );
			break;
		case Room::State::READY:
			break;
		case Room::State::PLAYING:
		{
			for ( auto& pair : mParticipants )
			{
				if ( false == pair.second.update() )
				{
					// Exception
					std::cerr << "FAIL: Client " << pair.first << "'s time out.\n";
					failedIndices.emplace_back( pair.first );
				}
				if ( Playing::UpdateResult::LINE_CLEARED == pair.second.updateResult() )
				{
					const uint8_t numOfLinesCleared = pair.second.numOfLinesCleared();
					const int32_t tempoDiff = TEMPO_DIFF_MS*numOfLinesCleared;
					pair.second.setRelativeTempoMs( tempoDiff );
					mHasTempoChanged_ = true;
				}
			}
			break;
		}
		case Room::State::ALL_OVER:
			for ( auto& pair : mParticipants )
			{
				mCandidateParticipants.emplace_back( pair.first );
				clients[pair.first].setState( Client::State::WAITING_IN_ROOM );
			}
			mParticipants.clear( );
			break;
		default:
#ifdef _DEBUG
			__debugbreak( );
#else
			__assume( 0 );
#endif
	}
	return failedIndices;
}

std::vector<ClientIndex> Room::notify( std::vector<Client>& clients )
{
	std::vector<ClientIndex> failedIndices;
	std::vector<ClientIndex> everyoneInRoom;
	for ( const ClientIndex idx : mCandidateParticipants )
	{
		everyoneInRoom.emplace_back( idx );
	}
	for ( const auto& pair : mParticipants )
	{
		everyoneInRoom.emplace_back( pair.first );
	}
	bool isAllOver = true;
	switch ( mState )
	{
		case Room::State::WAITING:
			break;
		case Room::State::ON_START:
		{
			Packet packet;
			const uint8_t ignored = 1;
			packet.pack( TAGGED_REQ_GET_READY, ignored );
			for ( auto& pair : mParticipants )
			{
				Socket& socket = clients[pair.first].socket();
				if ( -1 == socket.sendOverlapped(packet) )
				{
					std::cerr << "WARNING: Failed to tell that the game gets started to Client " <<
						pair.first << ".\n";
					failedIndices.emplace_back( pair.first );
				}
				pair.second.spawnTetrimino( );
			}
			mAlarms[(int)AlarmIndex::START] = Clock::now( );
			mState = Room::State::READY;
			break;
		}
		case Room::State::READY:
			if ( true == alarmAfter(COUNTDOWN_MS, AlarmIndex::START) )
			{
				std::string tetriminos;
				for ( const auto& pair : mParticipants )
				{
					const Playing& playing = pair.second;
					const ::model::tetrimino::Type curTetType = playing.currentTetriminoType();
					const ::model::tetrimino::Type nextTetType = playing.nextTetriminoType();
					const uint32_t tempoMs = playing.tempoMs();
					Packet packet;
					packet.pack( TAG_MY_NEXT_TETRIMINO, (uint8_t)nextTetType );
					packet.pack( TAG_MY_TEMPO_MS, tempoMs );
					Socket& socket = clients[pair.first].socket();
					if ( -1 == socket.sendOverlapped(packet) )
					{
						std::cerr << "WARNING: Failed to send to Client " << pair.first << ".\n";
						failedIndices.emplace_back( pair.first );
					}
					const HashedKey nicknameHashed = ::htonl(clients[pair.first].nicknameHashed());
					tetriminos.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
					tetriminos += (char)curTetType;
				}
				Packet packet;
				packet.pack( TAG_NEW_CURRENT_TETRIMINOS, tetriminos );
				for ( const ClientIndex idx : everyoneInRoom )
				{
					Socket& socket = clients[idx].socket();
					if ( -1 == socket.sendOverlapped(packet) )
					{
						std::cerr << "WARNING: Failed to send new tetriminos to Client " <<
							idx << ".\n";
						failedIndices.emplace_back( idx );
					}
				}
				mState = Room::State::PLAYING;
			}
			break;
		case Room::State::PLAYING:
		{
			std::string currentTetriminosMove;
			std::string newCurrentTetriminos;
			std::string stages;
			std::string numsOfLinesCleared;
			std::string gamesOver;
			for ( const auto& pair : mParticipants )
			{
				if ( true == isAllOver && false == pair.second.isGameOver() )
				{
					isAllOver = false;
				}
				const Playing::UpdateResult result = pair.second.updateResult();
				switch ( result )
				{
					case Playing::UpdateResult::NONE:
						break;
					case Playing::UpdateResult::TETRIMINO_MOVED:
					{
						const HashedKey nicknameHashed = ::htonl(clients[pair.first].nicknameHashed());
						const Playing& playing = pair.second;
						const ::model::tetrimino::Type curTetType = playing.currentTetriminoType();
						newCurrentTetriminos.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
						newCurrentTetriminos += (char)curTetType;
						const ::model::tetrimino::Rotation rotID = playing.currentTetriminoRotationID();
						const sf::Vector2<int8_t> pos( playing.currentTetriminoPosition() );
						currentTetriminosMove.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
						currentTetriminosMove += (char)rotID;
						currentTetriminosMove.append( (char*)&pos, sizeof(pos) );
						break;
					}
					case Playing::UpdateResult::TETRIMINO_LANDED:
					{
						const Playing& playing = pair.second;
						const ::model::tetrimino::Type nextTetType = playing.nextTetriminoType();
						std::string serializedStage( playing.serializedStage() );
						Packet packet;
						packet.pack( TAG_MY_NEXT_TETRIMINO, (uint8_t)nextTetType );
						packet.pack( TAG_MY_STAGE, serializedStage, false );
						Socket& socket = clients[pair.first].socket();
						if ( -1 == socket.sendOverlapped(packet) )
						{
							std::cerr << "WARNING: Failed to send to Client " << pair.first << ".\n";
							failedIndices.emplace_back( pair.first );
						}
						const HashedKey nicknameHashed = ::htonl(clients[pair.first].nicknameHashed());
						const ::model::tetrimino::Type curTetType = playing.currentTetriminoType();
						newCurrentTetriminos.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
						newCurrentTetriminos += (char)curTetType;
						stages.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
						stages.append( serializedStage.data(), serializedStage.size() );
						break;
					}
					case Playing::UpdateResult::LINE_CLEARED:
					{
						const Playing& playing = pair.second;
						const ::model::tetrimino::Type nextTetType = playing.nextTetriminoType();
						std::string serializedStage( playing.serializedStage() );
						const uint8_t numOfLinesCleared = playing.numOfLinesCleared();
						Packet packet;
						packet.pack( TAG_MY_NEXT_TETRIMINO, (uint8_t)nextTetType );
						packet.pack( TAG_MY_STAGE, serializedStage, false );
						Socket& socket = clients[pair.first].socket();
						if ( -1 == socket.sendOverlapped(packet) )
						{
							std::cerr << "WARNING: Failed to send to Client " << pair.first << ".\n";
							failedIndices.emplace_back( pair.first );
						}
						const HashedKey nicknameHashed = ::htonl(clients[pair.first].nicknameHashed());
						const ::model::tetrimino::Type curTetType = playing.currentTetriminoType();
						newCurrentTetriminos.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
						newCurrentTetriminos += (char)curTetType;
						stages.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
						stages.append( serializedStage.data(), serializedStage.size() );
						numsOfLinesCleared.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
						numsOfLinesCleared += (char)numOfLinesCleared;
						break;
					}
					case Playing::UpdateResult::GAME_OVER:
					{
						const Playing& playing = pair.second;
						std::string serializedStage( playing.serializedStage() );
						Packet packet;
						packet.pack( TAG_MY_STAGE, serializedStage, false );
						Socket& socket = clients[pair.first].socket();
						if ( -1 == socket.sendOverlapped(packet) )
						{
							std::cerr << "WARNING: Failed to send to Client " << pair.first << ".\n";
							failedIndices.emplace_back( pair.first );
						}
						const HashedKey nicknameHashed = ::htonl(clients[pair.first].nicknameHashed());
						stages.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
						stages.append( serializedStage.data(), serializedStage.size() );
						gamesOver.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
						break;
					}
					default:
#ifdef _DEBUG
						__debugbreak( );
#else
						__assume( 0 );
#endif
						break;
				}
				if ( true == mHasTempoChanged_ )
				{
					const uint32_t tempoMs = pair.second.tempoMs();
					Packet packet;
					packet.pack( TAG_MY_TEMPO_MS, tempoMs );
					Socket& socket = clients[pair.first].socket();
					if ( -1 == socket.sendOverlapped(packet) )
					{
						std::cerr << "WARNING: Failed to send to Client " << pair.first << ".\n";
						failedIndices.emplace_back( pair.first );
					}
				}
			}
			mHasTempoChanged_ = false;
			Packet packet;
			if ( false == currentTetriminosMove.empty() )
			{
				packet.pack( TAG_CURRENT_TETRIMINOS_MOVE, currentTetriminosMove );
			}
			if ( false == newCurrentTetriminos.empty() )
			{
				packet.pack( TAG_NEW_CURRENT_TETRIMINOS, newCurrentTetriminos );
			}
			if ( false == stages.empty() )
			{
				packet.pack( TAG_STAGES, stages );
			}
			if ( false == numsOfLinesCleared.empty() )
			{
				packet.pack( TAG_NUMS_OF_LINES_CLEARED, numsOfLinesCleared );
			}
			if ( false == gamesOver.empty() )
			{
				packet.pack( TAG_GAMES_OVER, gamesOver );
			}
			if ( true == isAllOver )
			{
				const uint8_t ignored = 1;
				packet.pack( TAG_ALL_OVER, ignored );
				mState = Room::State::ALL_OVER;
			}
			if ( true == packet.hasData() )
			{
				for ( const ClientIndex idx : everyoneInRoom )
				{
					Socket& socket = clients[idx].socket();
					if ( -1 == socket.sendOverlapped(packet) )
					{
						std::cerr << "WARNING: Failed to send to Client " << idx << ".\n";
						failedIndices.emplace_back( idx );
					}
				}
			}
			break;
		}
		case Room::State::ALL_OVER:
			mState = Room::State::WAITING;
			break;
		default:
#ifdef _DEBUG
			__debugbreak( );
#else
			__assume( 0 );
#endif
			break;
	}
	if ( true == alarmAfter(UPDATE_USER_LIST_INTERVAL_MS, AlarmIndex::UPDATE_USER_LIST) ||
		true == isAllOver )
	{
		std::string userList;
		userList.reserve( PARTICIPANT_CAPACITY*10 );
		for ( const ClientIndex idx : everyoneInRoom )
		{
			const std::string& nickname = clients[idx].nickname();
			const uint8_t nicknameLen = (uint8_t)nickname.size();
			userList += (char)nicknameLen;
			userList += nickname;
		}
		Packet packet;
		packet.pack( TAGGED_NOTI_UPDATE_USER_LIST, userList );
		if ( true == mHasHostChanged_ )
		{
			packet.pack( TAGGED_NOTI_HOST_CHANGED, clients[mHostIndex].nicknameHashed() );
			mHasHostChanged_ = false;
		}
		for ( const ClientIndex idx : everyoneInRoom )
		{
			Socket& socket = clients[idx].socket();
			if ( -1 == socket.sendOverlapped(packet) )
			{
				std::cerr << "WARNING: Failed to send the list of users in the room to Client " <<
					idx << ".\n";
				failedIndices.emplace_back( idx );
			}
		}
	}

	return failedIndices;
}

ClientIndex Room::hostIndex( ) const
{
	return mHostIndex;
}

bool Room::tryAccept( const ClientIndex index )
{
	bool isSuccessful = true;
	const uint8_t pop = (uint8_t)(mCandidateParticipants.size() + mParticipants.size());
	if ( PARTICIPANT_CAPACITY == pop )
	{
		isSuccessful = false;
	}
	else
	{
		mCandidateParticipants.emplace_back( index );
	}
	return isSuccessful;
}
