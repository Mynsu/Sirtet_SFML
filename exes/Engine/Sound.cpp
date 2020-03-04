#include "pch.h"
#include "Sound.h"

const uint8_t MAX_NUM_OF_BUFFERS = 3;
bool SFMLSound::IsInstantiated = false;

SFMLSound::SFMLSound( )
	: mPreviousVolumeBGM( 0 )
{
	ASSERT_TRUE( false == IsInstantiated );

	setBGMVolume( 2.f );
	setSFXVolume( 4.f );

	IsInstantiated = true;
}

SFMLSound::~SFMLSound( )
{
	IsInstantiated = false;
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