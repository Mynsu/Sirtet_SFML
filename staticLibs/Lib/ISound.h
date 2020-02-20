////
//  Commonly used in exes/Engine, dlls/Game.
////

#pragma once
#include <string>

class ISound
{
public:
	ISound( const ISound& ) = delete;
	void operator=( const ISound& ) = delete;
	virtual ~ISound( ) = default;

	// 0 to 100
	virtual void setBGMVolume( const float volume ) = 0;
	// 0 to 100
	virtual void setSFXVolume( const float volume ) = 0;
	virtual bool playBGM( const std::string& fileName, const bool isRepeated = false ) = 0;
	virtual void stopBGM( ) = 0;
	virtual bool playSFX( const std::string& fileName ) = 0;
protected:
	ISound( ) = default;
};