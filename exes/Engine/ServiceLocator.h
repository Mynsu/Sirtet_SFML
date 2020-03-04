#pragma once
#include "pch.h" // Included as a set of headers, not as pch.
#include <GameLib/IServiceLocator.h>
#include "Console.h"
#include "Sound.h"

class ServiceLocator final : public IServiceLocator
{
private:
	// �� �̻��� �ν��Ͻ��� ���� �� �����ϴ�.
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

	// NOTE: ��а� ���񽺸� ���Ƴ��� ���� ����
	// ����coalescing���� ���� ���� ���ϸ� ���ϱ� ���� ������ �� ������ �Ҵ����� �ʰ� �ֽ��ϴ�.
	// �� ������ �ڵ� �� �ٸ� �ٲٸ� ������ ���� ȿ�� ���� ������ �� �ֽ��ϴ�.
	// �� �̻��� �ν��Ͻ��� ���� �� �־� �̱��� ���ϰ��� ������ �ٸ��ϴ�.
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
	// NOTE: ���� �Լ��� ���� ȣ��� �� �ֵ��� ���ø� Ư��ȭ���� �ʾҽ��ϴ�.
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
	// NOTE: �ٸ� ���� ���̺귯���� �δ���� ���Ƴ��� �� �ְ�,
	// ����, �����, �׽��ÿ� �ʿ��� null ���񽺸� �� ���� �ֽ��ϴ�.
	std::unique_ptr<ISound> mSound;
	Console mConsole;
	std::unordered_map<HashedKey, Dword> mVault;
};

// NOTE: �̱��� ���ϰ� �޸� �����ڸ� ȣ���� �� �ֽ��ϴ�.
extern ::ServiceLocator gService;