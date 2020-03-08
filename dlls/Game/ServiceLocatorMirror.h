#pragma once
#include <GameLib/IGame.h>

class ServiceLocatorMirror final
{
	friend void _2943305454( const EngineComponents );
private:
	// �� �̻��� �ν��Ͻ��� ���� �� �����ϴ�.
	static bool IsInstantiated;
public:
	ServiceLocatorMirror( )
		: mPtr( nullptr )
	{
#ifdef _DEBUG
		if ( true == IsInstantiated )
		{
			__debugbreak( );
		}
#endif
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

	// NOTE: ���۷����� ��ȯ���� �ʰ� ���������� ����߽��ϴ�.
	IServiceLocator* operator()( )
	{
		return mPtr;
	}
private:
	IServiceLocator* mPtr;
};

// NOTE: �̱��� ���ϰ� �޸� �����ڸ� ȣ���� �� �ֽ��ϴ�.
extern ::ServiceLocatorMirror gService;