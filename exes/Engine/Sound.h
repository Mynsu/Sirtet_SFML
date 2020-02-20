#pragma once
#include "Lib/ISound.h"

class Sound final : public ISound
{
public:
	Sound( );
	~Sound( );

	// 0 to 100
	void setBGMVolume( const float volume ) override
	{
		mBGMPlayer.setVolume( volume );
	}
	// 0 to 100
	void setSFXVolume( const float volume ) override
	{
		mSFXPlayer.setVolume( volume );
	}
	bool playBGM( const std::string& fileName, const bool isRepeated = false ) override;
	void stopBGM( ) override
	{
		mBGMPlayer.stop( );
	}
	bool playSFX( const std::string& fileName ) override;
private:
	static bool IsInstantiated;
	static const uint8_t MAX_NUM_OF_BUFFERS = 3;
	sf::Sound mBGMPlayer, mSFXPlayer;
	sf::SoundBuffer mSoundBufferForBGM;
	std::unordered_map<HashedKey, sf::SoundBuffer> mSoundBuffersForSFX;
};