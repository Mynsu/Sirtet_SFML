#pragma once

class ServiceLocatorMirror final
{
	friend void _2943305454( const EngineComponents );
public:
	ServiceLocatorMirror( )
		: mPtr( nullptr )
	{
		ASSERT_FALSE( IsInstantiated );
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

	IServiceLocator* operator()( )
	{
		return mPtr;
	}
private:
	static bool IsInstantiated;
	IServiceLocator* mPtr;
};

extern ::ServiceLocatorMirror gService;