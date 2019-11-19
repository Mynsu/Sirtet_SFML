#pragma once

class ServiceLocatorMirror
{
	friend void _2943305454( const EngineComponents );
public:
	inline ServiceLocatorMirror( )
		: mPtr( nullptr )
	{
		ASSERT_FALSE( IsInstantiated );
		IsInstantiated = true;
	}
	inline ~ServiceLocatorMirror( )
	{
		mPtr = nullptr;
		IsInstantiated = false;
	}

	IServiceLocator& operator*( )
	{
		return *mPtr;
	}
private:
	static bool IsInstantiated;
	IServiceLocator* mPtr;
};

extern ::ServiceLocatorMirror glpService;