#include "pch.h"
#include "Audio.h"

bool Audio::IsInstantiated = false;
const uint8_t Audio::MAX_NUM_OF_BUFFERS;

Audio::Audio( )
{
	IsInstantiated = true;
	setBGMVolume( 10.f );
	setSFXVolume( 20.f );
}

Audio::~Audio( )
{
	IsInstantiated = false;
}

bool Audio::playBGM( const std::string& fileName, const bool isRepeated )
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

bool Audio::playSFX( const std::string& fileName )
{
	bool isLoadSuccessful = true;
	if ( MAX_NUM_OF_BUFFERS < (uint8_t)mSoundBuffersForSFX.size() )
	{
		mSoundBuffersForSFX.clear();
	}
	const HashedKey fileNameHashed = ::util::hash::Digest(fileName.data(), (uint8_t)fileName.size());
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