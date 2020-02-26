#include "pch.h"
#include "Sound.h"

bool Sound::IsInstantiated = false;
const uint8_t Sound::MAX_NUM_OF_BUFFERS;

Sound::Sound( )
{
	ASSERT_TRUE( false == IsInstantiated );
	IsInstantiated = true;
	setBGMVolume( 2.f );
	setSFXVolume( 4.f );
}

Sound::~Sound( )
{
	IsInstantiated = false;
}

bool Sound::playBGM( const std::string& fileName, const bool isRepeated )
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

bool Sound::playSFX( const std::string& fileName )
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
	mSFXPlayer.setBuffer( result.first->second );
	mSFXPlayer.play( );

	return isLoadSuccessful;
}