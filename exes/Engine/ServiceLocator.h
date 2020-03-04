#pragma once
#include "pch.h" // Included as a set of headers, not as pch.
#include <GameLib/IServiceLocator.h>
#include "Console.h"
#include "Sound.h"

class ServiceLocator final : public IServiceLocator
{
private:
	// 둘 이상의 인스턴스를 만들 수 없습니다.
	static bool IsInstantiated;
public:
	ServiceLocator( )
		: mSound( std::make_unique<SFMLSound>() )
	{
		ASSERT_TRUE( false == IsInstantiated );
		IsInstantiated = true;
	}
	ServiceLocator( const ServiceLocator& ) = delete;
	void operator=( const ServiceLocator& ) = delete;
	ServiceLocator( ServiceLocator&& ) = delete;
	~ServiceLocator( )
	{
		IsInstantiated = false;
	}

	// NOTE: 당분간 서비스를 갈아끼울 일이 없어
	// 통합coalescing으로 인한 성능 저하를 피하기 위해 변수를 힙 영역에 할당하지 않고 있습니다.
	// 이 파일의 코드 몇 줄만 바꾸면 언제든 연쇄 효과 없이 복구할 수 있습니다.
	// 둘 이상의 인스턴스를 만들 수 있어 싱글턴 패턴과는 여전히 다릅니다.
	// Accesses the console.  Reveals only functions the external files like .dll use.
	IConsole& console( ) override
	{
		return mConsole;
	}
	// Accesses the whole console.
	Console& _console( )
	{
		return mConsole;
	}
	// Accesses global variables.
	auto vault( ) -> std::unordered_map<HashedKey, Dword>& override
	{
		return mVault;
	}
	// NOTE: 가상 함수를 통해 호출될 수 있도록 템플릿 특수화하지 않았습니다.
	void provideSound( std::unique_ptr<ISound>& soundService ) override
	{
		mSound.reset( );
		mSound = std::move(soundService);
	}
	auto sound( ) -> ISound& override
	{
		return *mSound;
	}
	void release( )
	{
		// !IMPORTANT: MUST NOT get rid of this line.
		mConsole.release( );
	}
private:
	// NOTE: 다른 사운드 라이브러리로 부담없이 갈아끼울 수 있고,
	// 구현, 디버깅, 테스팅에 필요한 null 서비스를 둘 수도 있습니다.
	std::unique_ptr<ISound> mSound;
	Console mConsole;
	std::unordered_map<HashedKey, Dword> mVault;
};

// NOTE: 싱글턴 패턴과 달리 생성자를 호출할 수 있습니다.
extern ::ServiceLocator gService;