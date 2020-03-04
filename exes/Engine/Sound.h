#pragma once
#include <GameLib/ISound.h>

class SFMLSound final : public ISound
{
private:
	// �� �̻��� �ν��Ͻ��� ���� �� �����ϴ�.
	static bool IsInstantiated;
public:
	SFMLSound( );
	~SFMLSound( );

	// 0 to 100
	void setBGMVolume( const float volume ) override
	{
		mBGMPlayer.setVolume( volume );
	}
	// 0 to 100
	void setSFXVolume( const float volume ) override
	{
		for ( sf::Sound& player : mSFXPlayers )
		{
			player.setVolume( volume );
		}
	}
	void toggleBGMMute( ) override
	{
		if ( 0.f == mBGMPlayer.getVolume() )
		{
			mBGMPlayer.setVolume( mPreviousVolumeBGM );
		}
		else
		{
			mPreviousVolumeBGM = mBGMPlayer.getVolume();
			mBGMPlayer.setVolume( 0 );
		}
	}
	bool playBGM( std::string& fileName, const bool isRepeated = false ) override;
	void stopBGM( ) override
	{
		mBGMPlayer.stop( );
	}
	bool playSFX( std::string& fileName ) override;
private:
	static const uint8_t NUM_OF_SFX_PLAYERS = 2;
	float mPreviousVolumeBGM;
	sf::Sound mBGMPlayer, mSFXPlayers[NUM_OF_SFX_PLAYERS];
	sf::SoundBuffer mSoundBufferForBGM;
	std::unordered_map<HashedKey, sf::SoundBuffer> mSoundBuffersForSFX;
};