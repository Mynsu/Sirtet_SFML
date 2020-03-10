#include "pch.h"
#include "ServiceLocator.h"

bool ::ServiceLocator::IsInstantiated = false;
::ServiceLocator gService;

void ServiceLocator::_setVolume( const std::string_view& arg )
{
	const char BGM[] = "BGM";
	const char SFX[] = "SFX";
	if ( arg.size() < 5 ||
		0 != arg.compare(0, 3, BGM) && 0 != arg.compare(0, 3, SFX) ||
		arg[4] < 48 || 57 < arg[4] )
	{
		mConsole.printFailure(FailureLevel::WARNING, "vol [BGM/SFX] [0~100]");
		return;
	}

	::sound::Target target;
	if ( 0 == arg.compare(0, 3, BGM) )
	{
		target = ::sound::Target::BGM;
	}
	else
	{
		target = ::sound::Target::SFX;
	}
	const float volume = (float)std::atoi(&arg[4]);
	gService.sound().setVolume(target, volume);
}