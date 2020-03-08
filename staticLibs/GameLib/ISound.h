////
//  Shared by exes/Engine, dlls/Game.
////

#pragma once
#include <string>

namespace sound
{
	enum class Target
	{
		BGM,
		SFX,
	};
}

class ISound
{
public:
	ISound( const ISound& ) = delete;
	void operator=( const ISound& ) = delete;
	virtual ~ISound( ) = default;

	// If volume is 0.f to 100.f, sets absolute volume.
	// If volume is negative, sets relative volume.
	virtual void setVolume( const ::sound::Target target, const float volume ) = 0;
	virtual void toggleMute( const ::sound::Target target ) = 0;
	virtual bool playBGM( std::string& fileName, const bool isRepeated = false ) = 0;
	virtual void stopBGM( ) = 0;
	virtual bool playSFX( std::string& fileName ) = 0;
protected:
	ISound( ) = default;
};