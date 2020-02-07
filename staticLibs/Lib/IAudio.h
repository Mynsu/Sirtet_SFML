#pragma once
#include <string>

class IAudio
{
public:
	IAudio( const IAudio& ) = delete;
	void operator=( const IAudio& ) = delete;
	virtual ~IAudio( ) = default;

	// 0 <= volume <= 100
	virtual void setBGMVolume( const float volume ) = 0;
	virtual void setSFXVolume( const float volume ) = 0;
	virtual bool playBGM( const std::string& fileName, const bool isRepeated = false ) = 0;
	virtual void stopBGM( ) = 0;
	virtual bool playSFX( const std::string& fileName ) = 0;
protected:
	IAudio( ) = default;
};