#include "../../pch.h"
#include "InRoom.h"
#include "Online.h"
#include "../../ServiceLocatorMirror.h"
#include "../CommandList.h"
#include "../VaultKeyList.h"

scene::online::InRoom::InRoom( sf::RenderWindow& window, Online& net, const bool asHost )
	: mAsHost( asHost ), mIsReceiving( false ), mFrameCount( 0 ),
	mWindow_( window ), mNet( net )
{
	mFPS_ = (*glpService).vault( )[ HK_FORE_FPS ];
	const std::string& nickname = mNet.nickname( );
	mDigestedNickname_ = ::util::hash::Digest( nickname.data(), (uint8_t)nickname.size() );
	mParticipants.emplace( mDigestedNickname_, ::ui::StageView() );
	loadResources( );
#ifdef _DEBUG
	( *glpService ).console( )->print( "Now in a room.", sf::Color::Green );
#endif
#ifdef _DEV
	//(*glpService).console( )->addCommand( CMD_INVITE, std::bind(&scene::online::InRoom::invite,
	//															 this, std::placeholders::_1) );
	(*glpService).console( )->addCommand( CMD_LEAVE_ROOM, std::bind( &scene::online::InRoom::leaveRoom,
																 this, std::placeholders::_1 ) );
	if ( true == asHost )
	{
		(*glpService).console( )->addCommand( CMD_START_GAME, std::bind( &scene::online::InRoom::startGame,
																	 this, std::placeholders::_1 ) );
	}
#endif
}

scene::online::InRoom::~InRoom( )
{
#ifdef _DEV
	//(*glpService).console( )->removeCommand( CMD_INVITE );
	(*glpService).console( )->removeCommand( CMD_LEAVE_ROOM );
	if ( true == mAsHost )
	{
		(*glpService).console( )->removeCommand( CMD_START_GAME );
	}
#endif
}

void scene::online::InRoom::loadResources( )
{
	mBackgroundRect.setSize( sf::Vector2f(mWindow_.getSize()) );
	const sf::Color SKY( 0x8ae5ffff );
	mBackgroundRect.setFillColor( SKY );
	sf::Vector2f playerViewPos( 130.f, 0.f );
	float cellSize = 30.f;
	mParticipants[ mDigestedNickname_ ].setDimension( playerViewPos, cellSize );
}

::scene::online::ID scene::online::InRoom::update( std::list<sf::Event>& eventQueue )
{
	if ( false == mIsReceiving )
	{
		mNet.receive( );
		mIsReceiving = true;
	}
	
	::scene::online::ID retVal = ::scene::online::ID::AS_IS;
	if ( true == mNet.hasReceived() )
	{
		if ( std::optional<std::string> curTet(mNet.getByTag(TAG_CURRENT_TETRIMINO, Online::Option::SERIALIZED));
				  std::nullopt != curTet )
		{
			::model::tetrimino::Info* info = (::model::tetrimino::Info*)curTet.value( ).data( );
			///info->rotationID = (decltype(info->rotationID))::ntohl( (u_long)info->rotationID );
			info->type = (decltype(info->type))::ntohl( (u_long)info->type );
			mParticipants[ mDigestedNickname_ ].setCurrentTetrimino( *info );
		}
		else if ( std::optional<std::string> reqToGetReady(mNet.getByTag(TAG_GET_READY, Online::Option::RETURN_TAG_ATTACHED));
			 std::nullopt != reqToGetReady )
		{
			mFrameCount = mFPS_*-3;
		}
		mIsReceiving = false;
	}

	return retVal;

//	switch ( mState )
//	{
//		case State::DOING_NOTHING:
//			if ( true == mNet.hasReceived() )
//			{
//				if ( std::optional<std::string> res = mNet.getByTag(TAG_NOTIFY_JOINING);
//					 std::nullopt != res )
//				{
////TODO
//					mParticipants.emplace( (uint32_t)std::atoi(res.value().data()), ::model::Stage(mWindow_) );
//				}
//				else if ( std::optional<std::string> res = mNet.getByTag(TAG_OLDERS);
//						  std::nullopt != res )
//				{
//					std::string_view olders( res.value() );
//					size_t pos = TAG_OLDERS_LEN + 1u;
//					size_t nextPos = 0u;
//					while ( nextPos < olders.size() )
//					{
//						nextPos = olders.find_first_of( TOKEN_SEPARATOR_2, pos );
//						std::string one( olders.substr(pos, nextPos-pos) );
//						mParticipants.emplace( (uint32_t)std::atoi(one.data()), ::model::Stage(mWindow_) );
//						pos = ++nextPos;
//					}
//				}
//			}
//			break;
//		case State::INVITING:
//			if ( true == mNet.hasSent() )
//			{
//				mNet.receive( );
//			}
//			else if ( true == mNet.hasReceived() )
//			{
//				const char* const rcvBuf = mNet.receivingBuffer( );
////±Ã±Ý: Àß µÉ±î?				
//				if ( RESPONSE_NEGATION == rcvBuf[0] )
//				{
////TODO					
//					(*glpService).console( )->print( "There's no such ID", sf::Color::Green );
//				}
//				mState = State::DOING_NOTHING;
//			}
//			break;
//		default:
//#ifdef _DEBUG
//			__debugbreak( );
//#else
//			__assume( 0 );
//#endif
//	}
//	return retVal;
}

void scene::online::InRoom::draw( )
{
	mWindow_.draw( mBackgroundRect );
	for ( auto& it : mParticipants )
	{
		it.second.draw( mWindow_, mFrameCount/mFPS_ );
	}
	if ( 0 != mFrameCount )
	{
		++mFrameCount;
	}
}

//void scene::online::InRoom::invite( const std::string_view& arg )
//{
//	//TODO
//	if ( arg[0] < '0' || '9' < arg[0] )
//	{
//		// Exception
//		(*glpService).console( )->printFailure( FailureLevel::WARNING, "Unknown arguments." );
//		return;
//	}
//	std::string data( TAG_INVITE+std::to_string(std::atoi(arg.data())) );
//	mNet.send( data );
//}

void scene::online::InRoom::startGame( const std::string_view& arg )
{
	char req = TAG_START_GAME[ 0 ];
	mNet.send( &req, 1 );
}

void scene::online::InRoom::leaveRoom( const std::string_view & arg )
{
	mNet.disconnect( );
}

