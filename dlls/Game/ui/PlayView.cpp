#include "../pch.h"
#include "PlayView.h"
#include "../scene/online/Online.h"
#include "../ServiceLocatorMirror.h"

const uint8_t HARD_DROP_SPEED = 3;
const uint16_t INPUT_DELAY_MS = 40;
const uint16_t ASYNC_TOLERANCE_MS = 2500;
std::string ui::PlayView::SoundPaths[(int)SoundIndex::NONE_MAX];

void ui::PlayView::LoadResources( )
{
	SoundPaths[(int)SoundIndex::TETRIMINO_LOCK] = "Sounds/tetriminoLocked.wav";
	SoundPaths[(int)SoundIndex::LINE_CLEAR] = "Sounds/lineCleared.wav";

	lua_State* lua = luaL_newstate( );
	const std::string scriptPath( "Scripts/Playing.lua" );
	if ( true == luaL_dofile(lua, scriptPath.data()) )
	{
		gService()->console().printFailure( FailureLevel::FATAL,
											 "File Not Found: "+scriptPath );
	}
	else
	{
		luaL_openlibs( lua );
		const int TOP_IDX = -1;

		std::string tableName( "Sound" );
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK, tableName, scriptPath );
		}
		else
		{
			std::string innerTableName( "tetriminoLocked" );
			lua_pushstring( lua, innerTableName.data() );
			lua_gettable( lua, 1 );
			if ( false == lua_istable(lua, TOP_IDX) )
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+innerTableName, scriptPath );
			}
			else
			{
				std::string field( "path" );
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				int type = lua_type(lua, TOP_IDX);
				if ( LUA_TSTRING == type )
				{
					SoundPaths[(int)SoundIndex::TETRIMINO_LOCK] = lua_tostring(lua, TOP_IDX);
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+field, scriptPath );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+field, scriptPath );
				}
				lua_pop( lua, 1 );
			}
			lua_pop( lua, 1 );

			innerTableName = "lineCleared";
			lua_pushstring( lua, innerTableName.data() );
			lua_gettable( lua, 1 );
			if ( false == lua_istable(lua, TOP_IDX) )
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+innerTableName, scriptPath );
			}
			else
			{
				std::string field( "path" );
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				int type = lua_type(lua, TOP_IDX);
				if ( LUA_TSTRING == type )
				{
					SoundPaths[(int)SoundIndex::LINE_CLEAR] = lua_tostring(lua, TOP_IDX);
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+field, scriptPath );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+field, scriptPath );
				}
				lua_pop( lua, 1 );
			}
			lua_pop( lua, 1 );
		}
		lua_pop( lua, 1 );
	}
	lua_close( lua );

	::model::Tetrimino::LoadResources( );
}

ui::PlayView::PlayView( const bool isPlayable )
	: mHasTetriminoLandedOnClient( false ), mHasTetriminoLandedOnServer( false ),
	mIsForThisPlayer( isPlayable ), mHasCurrentTetrimino( false ),
	mCountDownSec( 3 ), mNumOfLinesCleared( 0 ),
	mFrameCountInputDelay( 0 ), mFrameCountVfxDuration( 0 ),
	mTempoMs( 1000 ),
	mState( State::WAITING_OR_OVER ),
	mAlarms{ Clock::time_point::max() },
	mTexture_countdown( std::make_unique<sf::Texture>() )
{ }

void ui::PlayView::update( std::vector<sf::Event>& eventQueue, ::scene::online::Online& net )
{
	if ( true == net.hasReceived() )
	{
		if ( const std::optional<std::string> tempoMs( net.getByTag(TAG_MY_TEMPO_MS,
																	::scene::online::Online::Option::DEFAULT,
																	sizeof(uint16_t)) );
			std::nullopt != tempoMs )
		{
			const uint16_t tempo = *(uint16_t*)tempoMs.value().data();
			mTempoMs = ::ntohs(tempo);
		}
	}

	// Keeping a tetrimino from doing soft-drop even though countdown hasn't ended yet.
	if ( State::ON_START == mState &&
		0 == mCountDownSec &&
		true == mHasCurrentTetrimino )
	{
		mState = State::PLAYING;
	}

	if ( State::PLAYING != mState )
	{
		return;
	}
	
	if ( true == mIsForThisPlayer )
	{
		// Exception
		if ( true == mHasTetriminoLandedOnServer &&
			true == alarmAfter(ASYNC_TOLERANCE_MS, AlarmIndex::GAP_LANDING_ON_SERVER) )
		{
			gService()->console().printFailure( FailureLevel::FATAL, "Over asynchronization." );
			net.disconnect( );
			return;
		}
		if ( false == mHasTetriminoLandedOnClient )
		{
			{
				auto& vault = gService()->vault();
				const auto it = vault.find(HK_HAS_GAINED_FOCUS);
				ASSERT_TRUE( vault.end() != it );
				if ( 1 == it->second )
				{
					const auto it2 = vault.find(HK_FORE_FPS);
					ASSERT_TRUE( vault.end() != it2 );
					mFPS_ = it2->second;
				}
				else
				{
					const auto it2 = vault.find(HK_BACK_FPS);
					ASSERT_TRUE( vault.end() != it2 );
					mFPS_ = it2->second;
				}
			}
			if ( true == mCurrentTetrimino.isHardDropping() )
			{
				for ( uint8_t i = 0; HARD_DROP_SPEED != i; ++i )
				{
					mHasTetriminoLandedOnClient = mCurrentTetrimino.moveDown(mStage.cgrid());
					if ( true == mHasTetriminoLandedOnClient )
					{
						mCurrentTetrimino.hardDrop( false );
						std::string msg( TAG_MY_TETRIMINO_LANDED_ON_CLIENT );
						net.send( msg.data(), (int)msg.size() );
						break;
					}
				}
				if ( false == mHasTetriminoLandedOnClient )
				{
					return;
				}
			}
			else
			{
				Packet packet;
				const uint16_t inputDelayFPS = mFPS_ * INPUT_DELAY_MS / 1000;
				for ( auto it = eventQueue.cbegin(); eventQueue.cend() != it; )
				{
					if ( sf::Event::KeyPressed == it->type )
					{
						switch ( it->key.code )
						{
							case sf::Keyboard::Space:
								if ( inputDelayFPS < mFrameCountInputDelay )
								{
									mFrameCountInputDelay = 0;
									if ( false == mHasTetriminoLandedOnServer )
									{
										packet.pack( TAG_MY_TETRIMINO_MOVE, (uint8_t)::model::tetrimino::Move::HARD_DROP );
									}
									mCurrentTetrimino.hardDrop( );
									resetAlarm( AlarmIndex::INTERVAL_TETRIMINO_DOWN );
								}
								it = eventQueue.erase(it);
								break;
							case sf::Keyboard::Down:
								if ( inputDelayFPS < mFrameCountInputDelay )
								{
									mFrameCountInputDelay = 0;
									if ( false == mHasTetriminoLandedOnServer )
									{
										packet.pack( TAG_MY_TETRIMINO_MOVE, (uint8_t)::model::tetrimino::Move::DOWN );
									}
									mHasTetriminoLandedOnClient = mCurrentTetrimino.moveDown( mStage.cgrid() );
									if ( true == mHasTetriminoLandedOnClient )
									{
										const uint8_t ignored = 1;
										packet.pack( TAG_MY_TETRIMINO_LANDED_ON_CLIENT, ignored );
									}
									resetAlarm( AlarmIndex::INTERVAL_TETRIMINO_DOWN );
								}
								it = eventQueue.erase(it);
								break;
							case sf::Keyboard::Left:
								if ( inputDelayFPS < mFrameCountInputDelay )
								{
									mFrameCountInputDelay = 0;
									if ( false == mHasTetriminoLandedOnServer )
									{
										packet.pack( TAG_MY_TETRIMINO_MOVE, (uint8_t)::model::tetrimino::Move::LEFT );
										mCurrentTetrimino.tryMoveLeft( mStage.cgrid() );
									}
								}
								it = eventQueue.erase(it);
								break;
							case sf::Keyboard::Right:
								if ( inputDelayFPS < mFrameCountInputDelay )
								{
									mFrameCountInputDelay = 0;
									if ( false == mHasTetriminoLandedOnServer )
									{
										packet.pack( TAG_MY_TETRIMINO_MOVE, (uint8_t)::model::tetrimino::Move::RIGHT );
										mCurrentTetrimino.tryMoveRight( mStage.cgrid() );
									}
								}
								it = eventQueue.erase(it);
								break;
							case sf::Keyboard::LShift:
								[[ fallthrough ]];
							case sf::Keyboard::Up:
								if ( inputDelayFPS < mFrameCountInputDelay )
								{
									mFrameCountInputDelay = 0;
									if ( false == mHasTetriminoLandedOnServer )
									{
										packet.pack( TAG_MY_TETRIMINO_MOVE, (uint8_t)::model::tetrimino::Move::ROTATE );
										mCurrentTetrimino.tryRotate( mStage.cgrid() );
									}
								}
								it = eventQueue.erase(it);
								break;
							default:
								++it;
								break;
						}
					}
					else
					{
						++it;
					}
				}

				if ( false == mHasTetriminoLandedOnClient &&
					true == alarmAfter(mTempoMs, AlarmIndex::INTERVAL_TETRIMINO_DOWN) )
				{
					mHasTetriminoLandedOnClient = mCurrentTetrimino.moveDown( mStage.cgrid() );
					if ( true == mHasTetriminoLandedOnClient )
					{
						const uint8_t ignored = 1;
						packet.pack( TAG_MY_TETRIMINO_LANDED_ON_CLIENT, ignored );
					}
					resetAlarm( AlarmIndex::INTERVAL_TETRIMINO_DOWN );
				}

				if ( true == packet.hasData() )
				{
					net.send( packet );
				}
			}
		}
	}

	if ( true == mHasTetriminoLandedOnClient ||
		false == mIsForThisPlayer )
	{
		if ( true == mHasTetriminoLandedOnServer )
		{
			mHasTetriminoLandedOnServer = false;
			mHasTetriminoLandedOnClient = false;
			if ( false == gService()->sound().playSFX(SoundPaths[(int)SoundIndex::TETRIMINO_LOCK]) )
			{
				gService()->console().printFailure(FailureLevel::WARNING,
												   "File Not Found: "+SoundPaths[(int)SoundIndex::TETRIMINO_LOCK] );
			}
			mCurrentTetrimino = mNextTetriminos.front();
			mNextTetriminos.pop_front( );
			mNextTetriminoPanel.setTetrimino( mNextTetriminos.front() );
			if ( true == mIsForThisPlayer )
			{
				mStage.deserialize( mBufferForStage );
			}
		}
	}
}

void ui::PlayView::draw( sf::RenderWindow& window )
{
	mStage.draw( window );
	if ( State::PLAYING == mState )
	{
		mCurrentTetrimino.draw( window );
	}
	else if ( State::ON_START == mState )
	{
		if ( 0 != mCountDownSec )
		{
			if ( true == alarmAfter(1000, AlarmIndex::COUNT_DOWN) )
			{
				--mCountDownSec;
				resetAlarm( AlarmIndex::COUNT_DOWN );
			}
			mSprite_countdown.setTextureRect( sf::IntRect(0, countdownSpriteSize.y*(mCountDownSec-1),
														  countdownSpriteSize.x, countdownSpriteSize.y) );
			window.draw( mSprite_countdown );
		}
	}
	if ( true == mIsForThisPlayer )
	{
		mNextTetriminoPanel.draw( window );
		if ( 0 < mFrameCountVfxDuration )
		{
			mVfxCombo.draw( window, mNumOfLinesCleared );
			--mFrameCountVfxDuration;
		}
		++mFrameCountInputDelay;
	}
}

void ui::PlayView::setCountdownSpriteDimension( const sf::Vector2f origin,
											   const float cellSize,
											   const sf::Vector2i clipSize )
{
	ASSERT_TRUE( 0.f <= origin.x && 0.f <= origin.y &&
				0.f < cellSize &&
				0 < clipSize.x && 0 < clipSize.y );
	mSprite_countdown.setOrigin( sf::Vector2f(clipSize)*.5f );
	const sf::Vector2f panelSize( ::model::stage::GRID_WIDTH*cellSize, ::model::stage::GRID_HEIGHT*cellSize );
	mSprite_countdown.setPosition( origin + panelSize*.5f );
	if ( (int)panelSize.x < clipSize.x )
	{
		mSprite_countdown.setScale( 0.5f, 0.5f );
	}
	countdownSpriteSize = clipSize;
}

void ui::PlayView::getReady( )
{
	mHasTetriminoLandedOnClient = mHasTetriminoLandedOnServer = false;
	mHasCurrentTetrimino = false;
	mCountDownSec = 3;
	mTempoMs = 1000;
	resetAlarm( AlarmIndex::COUNT_DOWN );
	while ( false == mNextTetriminos.empty() )
	{
		mNextTetriminos.pop_front( );
	}
	mState = State::ON_START;
}

void ui::PlayView::playLineClearEffects( const uint8_t numOfLinesCleared )
{
	ASSERT_TRUE( numOfLinesCleared <= ::model::tetrimino::BLOCKS_A_TETRIMINO );
	mNumOfLinesCleared = numOfLinesCleared;
	if ( false == gService()->sound().playSFX(SoundPaths[(int)SoundIndex::LINE_CLEAR]) )
	{
		gService()->console().printFailure(FailureLevel::WARNING,
										   "File Not Found: "+SoundPaths[(int)SoundIndex::LINE_CLEAR] );
	}
	mFrameCountVfxDuration = mFPS_;
}