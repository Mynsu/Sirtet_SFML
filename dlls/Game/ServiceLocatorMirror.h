#pragma once
#include <GameLib/IGame.h>

class ServiceLocatorMirror final
{
	friend void _2943305454( const EngineComponents );
private:
	// 둘 이상의 인스턴스를 만들 수 없습니다.
	static bool IsInstantiated;
public:
	ServiceLocatorMirror( )
		: mPtr( nullptr )
	{
		ASSERT_TRUE( false == IsInstantiated );
		IsInstantiated = true;
	}
	ServiceLocatorMirror( const ServiceLocatorMirror& ) = delete;
	void operator=( const ServiceLocatorMirror& ) = delete;
	ServiceLocatorMirror( ServiceLocatorMirror&& ) = delete;
	~ServiceLocatorMirror( )
	{
		mPtr = nullptr;
		IsInstantiated = false;
	}

	// NOTE: 레퍼런스를 반환하지 않고 포인터임을 명시했습니다.
	IServiceLocator* operator()( )
	{
		return mPtr;
	}
private:
	IServiceLocator* mPtr;
};

extern ::ServiceLocatorMirror gService;