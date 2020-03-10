#include "pch.h"
#include "Sound.h"
#include <Lib/ScriptLoader.h>

const uint8_t MAX_NUM_OF_BUFFERS = 3;
bool SFMLSound::IsInstantiated = false;

SFMLSound::SFMLSound( )
	: mPreviousVolumeBGM( 0.f ), mPreviousVolumeSFX( 0.f )
{
	ASSERT_TRUE( false == IsInstantiated );

	setVolume( ::sound::Target::BGM, 2.f );
	setVolume( ::sound::Target::SFX, 4.f );

	const std::string scriptPath( "Scripts/_OnlyDuringDev.lua" );
	const std::string varName0( "IsBGMOffOnStart" );
	auto result = ::util::script::LoadFromScript(scriptPath, varName0);
	const auto it = result.find(varName0);
	if ( result.end() != it )
	{
		if ( true == std::get<bool>(it->second) )
		{
			toggleMute(::sound::Target::BGM);
		}
	}

	IsInstantiated = true;
}

SFMLSound::~SFMLSound( )
{
	IsInstantiated = false;
}

void SFMLSound::setVolume( const ::sound::Target target, const float volume )
{
	switch ( target )
	{
		case ::sound::Target::BGM:
			if ( 0.f <= volume )
			{
				mBGMPlayer.setVolume( volume );
			}
			else
			{
				float targetVol = mBGMPlayer.getVolume() + volume;
				if ( targetVol < 0.f )
				{
					targetVol = 0.f;
				}
				mBGMPlayer.setVolume( targetVol );
			}
			break;
		case ::sound::Target::SFX:
			if ( 0.f <= volume )
			{
				for ( sf::Sound& player : mSFXPlayers )
				{
					player.setVolume( volume );
				}
			}
			else
			{
				float targetVol = mSFXPlayers[0].getVolume() + volume;
				if ( targetVol < 0.f )
				{
					targetVol = 0.f;
				}
				for ( sf::Sound& player : mSFXPlayers )
				{
					player.setVolume( targetVol );
				}
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

void SFMLSound::toggleMute( const ::sound::Target target )
{
	switch ( target )
	{
		case ::sound::Target::BGM:
			if ( 0.f == mBGMPlayer.getVolume() )
			{
				setVolume( ::sound::Target::BGM, mPreviousVolumeBGM );
			}
			else
			{
				mPreviousVolumeBGM = mBGMPlayer.getVolume();
				setVolume( ::sound::Target::BGM, 0.f );
			}
			break;
		case ::sound::Target::SFX:
		{
			if ( 0.f == mSFXPlayers[0].getVolume() )
			{
				setVolume( ::sound::Target::SFX, mPreviousVolumeSFX );
			}
			else
			{
				mPreviousVolumeSFX = mSFXPlayers[0].getVolume();
				setVolume( ::sound::Target::SFX, 0.f );
			}
			break;
		}
		default:
#ifdef _DEBUG
			__debugbreak( );
#else
			__assume( 0 );
#endif
	}
}

bool SFMLSound::playBGM( std::string& fileName, const bool isRepeated )
{
	bool isLoadSuccessful = true;
	if ( false == mSoundBufferForBGM.loadFromFile(fileName) )
	{
		isLoadSuccessful = false;
		return isLoadSuccessful;
	}
	mBGMPlayer.setBuffer( mSoundBufferForBGM );
	mBGMPlayer.setLoop( isRepeated );
	mBGMPlayer.play( );

	return isLoadSuccessful;
}

bool SFMLSound::playSFX( std::string& fileName )
{
	bool isLoadSuccessful = true;
	if ( MAX_NUM_OF_BUFFERS < (uint8_t)mSoundBuffersForSFX.size() )
	{
		mSoundBuffersForSFX.clear();
	}
	const HashedKey fileNameHashed = ::util::hash::Digest2(fileName);
	auto result = mSoundBuffersForSFX.emplace(fileNameHashed, sf::SoundBuffer());
	if ( true == result.second )
	{
		if ( false == result.first->second.loadFromFile(fileName) )
		{
			isLoadSuccessful = false;
			return isLoadSuccessful;
		}
	}
	for ( sf::Sound& player : mSFXPlayers )
	{
		if ( sf::Sound::Playing != player.getStatus() )
		{
			player.setBuffer( result.first->second );
			player.play( );
			break;
		}
	}

	return isLoadSuccessful;
}