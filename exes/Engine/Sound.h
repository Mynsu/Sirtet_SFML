#pragma once
#include <GameLib/ISound.h>

class SFMLSound final : public ISound
{
private:
	// 둘 이상의 인스턴스를 만들 수 없습니다.
	static bool IsInstantiated;
public:
	SFMLSound( );
	~SFMLSound( );

	void setVolume( const ::sound::Target target, const float volume ) override;
	void toggleMute( const ::sound::Target target ) override;
	bool playBGM( std::string& fileName, const bool isRepeated = false ) override;
	void stopBGM( ) override
	{
		mBGMPlayer.stop( );
	}
	bool playSFX( std::string& fileName ) override;
private:
	static const uint8_t NUM_OF_SFX_PLAYERS = 2;
	float mPreviousVolumeBGM, mPreviousVolumeSFX;
	sf::Sound mBGMPlayer, mSFXPlayers[NUM_OF_SFX_PLAYERS];
	sf::SoundBuffer mSoundBufferForBGM;
	std::unordered_map<HashedKey, sf::SoundBuffer> mSoundBuffersForSFX;
};