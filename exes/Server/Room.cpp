#include "pch.h"
#include "Room.h"
#include "Client.h"

const uint16_t UPDATE_USER_LIST_INTERVAL_MS = 1000;
const uint16_t COUNTDOWN_MS = 3000;
const int16_t TEMPO_DIFF_MS = -20;

Room::Room( const ClientIndex hostIndex )
	: mHasTempoChanged_( false ), mHasHostChanged_( false ),
	mHostIndex( hostIndex ), mState( State::WAITING )
{
	mPlayingParticipants.reserve( PARTICIPANT_CAPACITY );
	mObservers.emplace_back( hostIndex, false );
	const Clock::time_point init = Clock::now();
	for ( Clock::time_point& alarm : mAlarms )
	{
		alarm = init;
	}
}

int8_t Room::pop( const ClientIndex index )
{
	bool isThereSomeoneRemains = true;
	bool wasCandidateParticipant = false;
	for ( auto it = mObservers.begin(); mObservers.end() != it; ++it )
	{
		if ( index == it->index )
		{
			mObservers.erase( it );
			wasCandidateParticipant = true;
#ifdef _DEV
			std::cout << "Observer " << index << " left the room.\n";
#endif
			break;
		}
	}
	if ( false == wasCandidateParticipant )
	{
#ifdef _DEV
		std::cout << "Observer " << index << " left the room.\n";
#endif
#ifdef _DEBUG
		ASSERT_TRUE( 1 == mPlayingParticipants.erase(index) );
#else
		mPlayingParticipants.erase( index );
#endif
	}

	if ( const uint8_t pop = (uint8_t)(mObservers.size()+mPlayingParticipants.size());
		0 == pop )
	{
		isThereSomeoneRemains = false;
	}
	else if ( index == mHostIndex )
	{
		if ( const auto it = mPlayingParticipants.cbegin();
			mPlayingParticipants.cend() != it )
		{
			mHostIndex = it->first;
		}
		else
		{
			const auto it2 = mObservers.cbegin();
			mHostIndex = it2->index;
		}
		mHasHostChanged_ = true;
	}
	return (true==isThereSomeoneRemains)? 1: 0;
}

void Room::perceive( const ClientIndex index, const Perception perceptedThing )
{
	switch ( perceptedThing )
	{
		case Perception::INSTANTIATION_DONE:
		{
			for ( Observer& ob : mObservers )
			{
				if ( index == ob.index )
				{
					if ( Observer::State::ZERO == ob.state )
					{
						ob.state = Observer::State::INSTANTIATION_DONE;
						break;
					}
#ifdef _DEV
					else
					{
						std::cerr << "Client " << index << " sent needless data(1).\n";
					}
#endif
				}
			}
			break;
		}
		case Perception::INITIATION_DONE:
		{
			for ( Observer& ob : mObservers )
			{
				if ( index == ob.index )
				{
					if ( Observer::State::INSTANTIATION_DONE == ob.state )
					{
						ob.state = Observer::State::DONE;
						break;
					}
#ifdef _DEV
					else
					{
						std::cerr << "Client " << index << " sent needless data(2).\n";
					}
#endif
				}
			}
			break;
		}
		case Perception::TETRIMINO_LAND_ON_CLIENT:
			if ( auto it = mPlayingParticipants.find(index); mPlayingParticipants.end() != it )
			{
				it->second.perceive( );
			}
			// Exception
			else
			{
				std::cerr << "Client " << index << " knows the incorrect room ID.\n";
			}
			break;
		default:
#ifdef _DEBUG
			__debugbreak( );
#else
			__assume( 0 );
#endif
	}
}

std::vector<ClientIndex> Room::update( std::array<Client, CLIENT_CAPACITY>& clients )
{
	std::vector<ClientIndex> failedIndices;
	switch ( mState )
	{
		case Room::State::WAITING:
			break;
		case Room::State::ON_START:
			for ( const Observer ob : mObservers )
			{
				auto pair = mPlayingParticipants.try_emplace(ob.index);
				pair.first->second.spawnTetriminos( );
				clients[ob.index].setState( Client::State::PLAYING_IN_ROOM );
			}
			mObservers.clear( );
			break;
		case Room::State::READY:
			break;
		case Room::State::PLAYING:
		{
			for ( auto& pair : mPlayingParticipants )
			{
				if ( false == pair.second.update() )
				{
					// Exception
					std::cerr << "FAIL: Client " << pair.first << "'s time out.\n";
					failedIndices.emplace_back( pair.first );
				}
				if ( Playing::UpdateResult::LINE_CLEARED == pair.second.updateResultToNotify() )
				{
					const uint8_t numOfLinesCleared = pair.second.numOfLinesClearedRecently();
					const int32_t tempoDiff = TEMPO_DIFF_MS*numOfLinesCleared;
					pair.second.setRelativeTempoMs( tempoDiff );
					mHasTempoChanged_ = true;
				}
			}
			break;
		}
		case Room::State::ALL_OVER:
			for ( auto& pair : mPlayingParticipants )
			{
				mObservers.emplace_back( pair.first, false );
				clients[pair.first].setState( Client::State::WAITING_IN_ROOM );
			}
			mPlayingParticipants.clear( );
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

std::vector<ClientIndex> Room::notify( std::array<Client, CLIENT_CAPACITY>& clients )
{
	std::vector<ClientIndex> failedIndices;
	std::vector<ClientIndex> everyoneInRoom;
	for ( const Observer ob : mObservers )
	{
		if ( Observer::State::DONE == ob.state )
		{
			everyoneInRoom.emplace_back( ob.index );
		}
	}
	for ( const auto& pair : mPlayingParticipants )
	{
		everyoneInRoom.emplace_back( pair.first );
	}
	switch ( mState )
	{
		case Room::State::WAITING:
			break;
		case Room::State::ON_START:
		{
			Packet packet;
			const uint8_t ignored = 1;
			packet.pack( TAGGED_REQ_GET_READY, ignored );
			for ( auto& pair : mPlayingParticipants )
			{
				Socket& socket = clients[pair.first].socket();
				if ( -1 == socket.sendOverlapped(packet) )
				{
					std::cerr << "WARNING: Failed to tell that the game gets started to Client " <<
						pair.first << ".\n";
					failedIndices.emplace_back( pair.first );
				}
			}
			mAlarms[(int)AlarmIndex::START] = Clock::now( );
			mState = Room::State::READY;
			break;
		}
		case Room::State::READY:
			if ( true == alarmAfterAndReset(COUNTDOWN_MS, AlarmIndex::START) )
			{
				std::string currentTetriminos;
				std::string nextTetriminos;
				for ( const auto& pair : mPlayingParticipants )
				{
					const Playing& playing = pair.second;
					const uint16_t tempoMs = playing.tempoMs();
					Packet packet;
					packet.pack( TAG_MY_TEMPO_MS, tempoMs );
					Socket& socket = clients[pair.first].socket();
					if ( -1 == socket.sendOverlapped(packet) )
					{
						std::cerr << "WARNING: Failed to send to Client " << pair.first << ".\n";
						failedIndices.emplace_back( pair.first );
					}
					const HashedKey nicknameHashed = ::htonl(clients[pair.first].nicknameHashed());
					currentTetriminos.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
					nextTetriminos.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
					const ::model::tetrimino::Type curTetType = playing.currentTetriminoType();
					currentTetriminos += (char)curTetType;
					const ::model::tetrimino::Type nextTetType = playing.nextTetriminoType();
					nextTetriminos += (char)nextTetType;
				}
				Packet packet;
				packet.pack( TAG_CURRENT_TETRIMINOS, currentTetriminos );
				packet.pack( TAG_NEXT_TETRIMINOS, nextTetriminos );
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
			bool isAllOver = true;
			std::string currentTetriminosMove;
			std::string currentTetriminosLand;
			std::string nextTetriminos;
			std::string stages;
			std::string numsOfLinesCleared;
			std::string gamesOver;
			for ( const auto& pair : mPlayingParticipants )
			{
				const HashedKey nicknameHashed = ::htonl(clients[pair.first].nicknameHashed());
				const Playing& playing = pair.second;
				if ( true == isAllOver && false == pair.second.isGameOver() )
				{
					isAllOver = false;
				}
				const Playing::UpdateResult result = pair.second.updateResultToNotify();
				switch ( result )
				{
					case Playing::UpdateResult::NONE:
						break;
					case Playing::UpdateResult::TETRIMINO_MOVED:
					{
						currentTetriminosMove.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
						const ::model::tetrimino::Rotation rotID = playing.currentTetriminoRotationID();
						currentTetriminosMove += (char)rotID;
						const sf::Vector2<int8_t> pos( playing.currentTetriminoPosition() );
						currentTetriminosMove.append( (char*)&pos, sizeof(pos) );
						break;
					}
					case Playing::UpdateResult::TETRIMINO_LANDED:
					{
						currentTetriminosLand.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
						nextTetriminos.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
						stages.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
						const ::model::tetrimino::Type nextTetType = playing.nextTetriminoType();
						nextTetriminos += (char)nextTetType;
						const ::model::stage::Grid& stage = playing.serializedStage();
						stages.append( (char*)std::addressof(stage), sizeof(stage) );
						break;
					}
					case Playing::UpdateResult::LINE_CLEARED:
					{
						currentTetriminosLand.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
						nextTetriminos.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
						stages.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
						numsOfLinesCleared.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
						const ::model::tetrimino::Type nextTetType = playing.nextTetriminoType();
						nextTetriminos += (char)nextTetType;
						const ::model::stage::Grid& stage = playing.serializedStage();
						stages.append( (char*)std::addressof(stage), sizeof(stage) );
						const uint8_t numOfLinesCleared = playing.numOfLinesClearedRecently();
						numsOfLinesCleared += (char)numOfLinesCleared;
						break;
					}
					case Playing::UpdateResult::GAME_OVER:
					{
						stages.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
						const ::model::stage::Grid& stage = playing.serializedStage();
						stages.append( (char*)std::addressof(stage), sizeof(stage) );
						gamesOver.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
						break;
					}
					default:
#ifdef _DEBUG
						__debugbreak( );
#else
						__assume( 0 );
#endif
				}
				if ( true == mHasTempoChanged_ )
				{
					const uint16_t tempoMs = pair.second.tempoMs();
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
			if ( false == currentTetriminosLand.empty() )
			{
				packet.pack( TAG_CURRENT_TETRIMINOS_LAND, currentTetriminosLand );
			}
			if ( false == currentTetriminosMove.empty() )
			{
				packet.pack( TAG_CURRENT_TETRIMINOS_MOVE, currentTetriminosMove );
			}
			if ( false == nextTetriminos.empty() )
			{
				packet.pack( TAG_NEXT_TETRIMINOS, nextTetriminos );
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
	}
	if ( true == alarmAfterAndReset(UPDATE_USER_LIST_INTERVAL_MS, AlarmIndex::UPDATE_USER_LIST) )
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
		bool hasNewObserver = false;
		for ( const Observer ob : mObservers )
		{
			if ( Observer::State::DONE != ob.state )
			{
				if ( Observer::State::INSTANTIATION_DONE == ob.state )
				{
					hasNewObserver = true;
				}
				const std::string& nickname = clients[ob.index].nickname();
				const uint8_t nicknameLen = (uint8_t)nickname.size();
				userList += (char)nicknameLen;
				userList += nickname;
			}
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
		if ( true == hasNewObserver )
		{
			std::string currentTetriminos, nextTetriminos, stages, gamesOver;
			for ( const auto& pair : mPlayingParticipants )
			{
				const Playing& playing = pair.second;
				const HashedKey nicknameHashed = ::htonl(clients[pair.first].nicknameHashed());
				currentTetriminos.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
				nextTetriminos.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
				stages.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
				if ( true == playing.isGameOver() )
				{
					gamesOver.append( (char*)&nicknameHashed, sizeof(nicknameHashed) );
				}
				const ::model::tetrimino::Type curTetType = playing.currentTetriminoType();
				currentTetriminos += (char)curTetType;
				const ::model::tetrimino::Type nextTetType = playing.nextTetriminoType();
				nextTetriminos += (char)nextTetType;
				const ::model::stage::Grid& stage = playing.serializedStage();
				stages.append( (char*)std::addressof(stage), sizeof(stage) );
			}
			if ( false == currentTetriminos.empty() )
			{
				packet.pack( TAG_CURRENT_TETRIMINOS, currentTetriminos );
			}
			if ( false == nextTetriminos.empty() )
			{
				packet.pack( TAG_NEXT_TETRIMINOS, nextTetriminos );
			}
			if ( false == stages.empty() )
			{
				packet.pack( TAG_STAGES, stages );
			}
			if ( false == gamesOver.empty() )
			{
				packet.pack( TAG_GAMES_OVER, gamesOver );
			}
			for ( Observer& ob : mObservers )
			{
				if ( Observer::State::INSTANTIATION_DONE == ob.state )
				{
					Socket& socket = clients[ob.index].socket();
					if ( -1 == socket.sendOverlapped(packet) )
					{
						std::cerr << "WARNING: Failed to send the list of users in the room to Client " <<
							ob.index << ".\n";
						failedIndices.emplace_back( ob.index );
					}
					ob.state = Observer::State::DONE;
				}
			}
		}
	}

	return failedIndices;
}

bool Room::tryEmplace( const ClientIndex index )
{
	bool isSuccessful = true;
	const uint8_t pop = (uint8_t)(mObservers.size() + mPlayingParticipants.size());
	if ( PARTICIPANT_CAPACITY == pop )
	{
		isSuccessful = false;
	}
	else
	{
		if ( State::PLAYING == mState )
		{
			mObservers.emplace_back( index, true );
		}
		else
		{
			mObservers.emplace_back( index, false );
		}
	}
	return isSuccessful;
}