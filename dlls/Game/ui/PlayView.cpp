#include "../pch.h"
#include "PlayView.h"
#include <Lib/VaultKeyList.h>
#include "../scene/online/Online.h"
#include "../ServiceLocatorMirror.h"

const uint8_t HARD_DROP_SPEED = 3;
const uint16_t INPUT_DELAY_MS = 40;
const uint16_t ASYNC_TOLERANCE_MS = 2500;
std::string ui::PlayView::SoundPaths[(int)SoundIndex::NONE_MAX];

void ui::PlayView::LoadResources( )
{
	SoundPaths[(int)SoundIndex::TETRIMINO_LOCKED] = "Sounds/tetriminoLocked.wav";
	SoundPaths[(int)SoundIndex::LINE_CLEARED] = "Sounds/lineCleared.wav";

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
					SoundPaths[(int)SoundIndex::TETRIMINO_LOCKED] = lua_tostring(lua, TOP_IDX);
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
					SoundPaths[(int)SoundIndex::LINE_CLEARED] = lua_tostring(lua, TOP_IDX);
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

ui::PlayView::PlayView( sf::RenderWindow& window, ::scene::online::Online& net, const bool isPlayable )
	: mHasTetriminoLandedOnClient( false ), mHasTetriminoLandedOnServer( false ),
	mIsForThisPlayer( isPlayable ), mHasCurrentTetrimino( false ),
	mCountDownSec( 3 ), mNumOfLinesCleared( 0 ),
	mFrameCountInputDelay( 0 ), mFrameCountVfxDuration( 0 ),
	mTempoMs( 1000 ),
	mState_( PlayView::State::WAITING_OR_OVER ),
	mWindow_( window ), mNet( net ),
	mAlarms{ Clock::time_point::max() },
	mTexture_countdown( std::make_unique<sf::Texture>() ), mVfxCombo( window ),
	mStage( window ), mNextTetriminoPanel( window )
{ }

ui::PlayView::PlayView( const PlayView& another )
	: mHasTetriminoLandedOnClient( false ), mHasTetriminoLandedOnServer( false ),
	mIsForThisPlayer( another.mIsForThisPlayer ), mHasCurrentTetrimino( false ),
	mCountDownSec( 0 ), mNumOfLinesCleared( 0 ),
	mFrameCountInputDelay( 0 ), mFrameCountVfxDuration( 0 ),
	mTempoMs( 1000 ),
	mState_( PlayView::State::WAITING_OR_OVER ),
	mWindow_( another.mWindow_ ), mNet( another.mNet ),
	mAlarms{ Clock::time_point::max() },
	mTexture_countdown( std::make_unique<sf::Texture>() ), mVfxCombo( mWindow_ ),
	mStage( mWindow_ ), mNextTetriminoPanel( mWindow_ )
{
}

bool ui::PlayView::loadCountdownSprite( std::string& filePathNName )
{
	if ( false == mTexture_countdown->loadFromFile(filePathNName) )
	{
		return false;
	}
	else
	{
		mSprite_countdown.setTexture( *mTexture_countdown );
		return true;
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
	countdownSpriteSize_ = clipSize;
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
		mNextTetriminos.pop( );
	}
	mState_ = PlayView::State::ON_START;
}

void ui::PlayView::update( std::vector<sf::Event>& eventQueue )
{
	// Exception
	if ( true == mHasTetriminoLandedOnServer &&
		true == alarmAfter(ASYNC_TOLERANCE_MS, AlarmIndex::LANDED_ON_SERVER) )
	{
		gService()->console().printFailure( FailureLevel::FATAL, "Over asynchronization." );
		mNet.disconnect( );
		return;
	}

	if ( true == mNet.hasReceived() )
	{
		if ( const std::optional<std::string> nextTet( mNet.getByTag(TAG_MY_NEXT_TETRIMINO,
																	 ::scene::online::Online::Option::DEFAULT,
																	 sizeof(uint8_t)) );
			std::nullopt != nextTet	)
		{
			const ::model::tetrimino::Type type =
				(::model::tetrimino::Type)*nextTet.value().data();
			mNextTetriminos.emplace( ::model::Tetrimino::Spawn(type) );
			if ( PlayView::State::ON_START == mState_ )
			{
				mNextTetriminoPanel.setTetrimino( mNextTetriminos.front() );
			}
		}

		if ( const std::optional<std::string> tempoMs( mNet.getByTag(TAG_MY_TEMPO_MS,
																	::scene::online::Online::Option::DEFAULT,
																  sizeof(uint16_t)) );
			std::nullopt != tempoMs )
		{
			const uint16_t tempo = *(uint16_t*)tempoMs.value().data();
			mTempoMs = ::ntohs(tempo);
		}
	}

	if ( PlayView::State::ON_START == mState_ &&
		0 == mCountDownSec &&
		true == mHasCurrentTetrimino )
	{
		mState_ = PlayView::State::PLAYING;
	}

	if ( false == mIsForThisPlayer ||
		PlayView::State::PLAYING != mState_ )
	{
		return;
	}
	
	Packet packet;
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
					const uint8_t ignored = 1;
					packet.pack( TAG_MY_TETRIMINO_LANDED_ON_CLIENT, ignored );
					goto last;
				}
			}
			return;
		}
		else
		{
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
								resetAlarm( AlarmIndex::TETRIMINO_DOWN );
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
								resetAlarm( AlarmIndex::TETRIMINO_DOWN );
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
				true == alarmAfter(mTempoMs, AlarmIndex::TETRIMINO_DOWN) )
			{
				mHasTetriminoLandedOnClient = mCurrentTetrimino.moveDown( mStage.cgrid() );
				if ( true == mHasTetriminoLandedOnClient )
				{
					const uint8_t ignored = 1;
					packet.pack( TAG_MY_TETRIMINO_LANDED_ON_CLIENT, ignored );
				}
				resetAlarm( AlarmIndex::TETRIMINO_DOWN );
			}
		}
	}

	last:
	if ( true == mHasTetriminoLandedOnClient )
	{
		if ( true == mHasTetriminoLandedOnServer )
		{
			if ( false == gService()->sound().playSFX(SoundPaths[(int)SoundIndex::TETRIMINO_LOCKED]) )
			{
				gService()->console().printFailure(FailureLevel::WARNING,
												   "File Not Found: "+SoundPaths[(int)SoundIndex::TETRIMINO_LOCKED] );
			}
			mHasTetriminoLandedOnServer = false;
			mHasTetriminoLandedOnClient = false;
			mCurrentTetrimino = mNextTetriminos.front();
			mNextTetriminos.pop( );
			mStage.deserialize( mBufferForStage );
			mNextTetriminoPanel.setTetrimino( mNextTetriminos.front() );
		}
#ifdef _DEBUG
		else
		{
			gService()->console().print( "Client has it done while Server hasn't it done yet.",
										sf::Color::Green );
		}
#endif
	}

	if ( true == packet.hasData() )
	{
		mNet.send( packet );
	}
}

void ui::PlayView::setNewCurrentTetrimino( const::model::tetrimino::Type newCurrentType )
{
	if ( false == mIsForThisPlayer ||
		false == mHasCurrentTetrimino )
	{
		mCurrentTetrimino = ::model::Tetrimino::Spawn(newCurrentType);
		mHasCurrentTetrimino = true;
		mState_ = State::PLAYING;
	}
}

void ui::PlayView::updateStage( const::model::stage::Grid& grid )
{
	if ( PlayView::State::WAITING_OR_OVER == mState_ ||
		false == mIsForThisPlayer )
	{
		mStage.deserialize( grid );
	}
	else
	{
		mBufferForStage = grid;
		resetAlarm( AlarmIndex::LANDED_ON_SERVER );
		mHasTetriminoLandedOnServer = true;
	}
}

void ui::PlayView::setNumOfLinesCleared( const uint8_t numOfLinesCleared )
{
	ASSERT_TRUE( numOfLinesCleared <= ::model::tetrimino::BLOCKS_A_TETRIMINO );
	mNumOfLinesCleared = numOfLinesCleared;
	if ( false == gService()->sound().playSFX(SoundPaths[(int)SoundIndex::LINE_CLEARED]) )
	{
		gService()->console().printFailure(FailureLevel::WARNING,
										   "File Not Found: "+SoundPaths[(int)SoundIndex::LINE_CLEARED] );
	}
	mFrameCountVfxDuration = mFPS_;
}

void ui::PlayView::gameOver( )
{
	mState_ = PlayView::State::WAITING_OR_OVER;
	mNextTetriminoPanel.clearTetrimino( );
}

void ui::PlayView::draw( )
{
	mStage.draw( );
	if ( true == mIsForThisPlayer )
	{
		mNextTetriminoPanel.draw( );
	}
	if ( PlayView::State::PLAYING == mState_ )
	{
		mCurrentTetrimino.draw( mWindow_ );
		if ( true == mIsForThisPlayer )
		{
			if ( 0 < mFrameCountVfxDuration )
			{
				mVfxCombo.draw( mNumOfLinesCleared );
				--mFrameCountVfxDuration;
			}
			++mFrameCountInputDelay;
		}
	}
	else if ( PlayView::State::ON_START == mState_ )
	{
		if ( 0 != mCountDownSec )
		{
			if ( true == alarmAfter(1000, AlarmIndex::COUNT_DOWN) )
			{
				--mCountDownSec;
				resetAlarm( AlarmIndex::COUNT_DOWN );
			}
			mSprite_countdown.setTextureRect( sf::IntRect(0, countdownSpriteSize_.y*(mCountDownSec-1),
												countdownSpriteSize_.x, countdownSpriteSize_.y) );
			mWindow_.draw( mSprite_countdown );
		}
	}
}

::model::Tetrimino& ui::PlayView::currentTetrimino( )
{
	return mCurrentTetrimino;
}

::model::Stage& ui::PlayView::stage( )
{
	return mStage;
}

::vfx::Combo& ui::PlayView::vfxCombo( )
{
	return mVfxCombo;
}

::ui::NextTetriminoPanel& ui::PlayView::nextTetriminoPanel( )
{
	return mNextTetriminoPanel;
}