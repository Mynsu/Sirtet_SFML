#include "../pch.h"
#include "SceneManager.h"
#include <Lib/ScriptLoader.h>
#include "../ServiceLocatorMirror.h"
#include "Intro.h"
#include "MainMenu.h"
#include "inPlay/InPlay.h"
#include "online/Online.h"

bool ::scene::SceneManager::IsInstantiated = false;

void ::scene::SceneManager::init( sf::RenderWindow& window )
{
	ASSERT_NOT_NULL( gService() );
	mWindow = &window;
#ifdef _DEV
	gService()->console().addCommand( CMD_CHANGE_SCENE, std::bind(&SceneManager::chscnto, this, std::placeholders::_1) );
	gService()->console().addCommand( CMD_RELOAD, std::bind(&SceneManager::refresh, this, std::placeholders::_1) );

	::scene::ID startScene = ::scene::ID::INTRO;
	const std::string scriptPath( "Scripts/_OnlyDuringDev.lua" );
	const std::string varName( "StartScene" );
	const auto result = ::util::script::LoadFromScript(scriptPath, varName);
	if ( const auto it = result.find(varName);
		result.cend() != it )
	{
		// Type check
		if ( true == std::holds_alternative<int>(it->second) )
		{
			startScene = (::scene::ID)std::get<int>(it->second);
		}
		else
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK, varName, scriptPath );
		}
	}
	setScene( startScene );
#else
	// NOTE: equals setScene( ::scene::ID::INTRO ).
	mCurrentScene = std::make_unique<::scene::Intro>(window);
#endif
}

void ::scene::SceneManager::setScene( const ::scene::ID nextScene )
{
	mCurrentScene.reset( nullptr );
	switch ( nextScene )
	{
		case ::scene::ID::INTRO:
			mCurrentScene = std::make_unique<::scene::Intro>(*mWindow);
			break;
		case ::scene::ID::MAIN_MENU:
			mCurrentScene = std::make_unique<::scene::MainMenu>(*mWindow);
			break;
		case ::scene::ID::SINGLE_PLAY:
			mCurrentScene = std::make_unique<::scene::inPlay::InPlay>(*mWindow);
			break;
		case ::scene::ID::ONLINE_BATTLE:
			mCurrentScene = std::make_unique<::scene::online::Online>(*mWindow);
			break;
		default:
			// 싱글 플레이 시퀀스에서 몇몇 씬을 건너뛸 경우,
			if ( (int)nextScene/10 == (int)::scene::ID::SINGLE_PLAY )
			{
				// 실행 직후거나 다른 시퀀스에 있어 싱글 플레이 시퀀스에 새로 진입하는 경우,
				if ( nullptr == mCurrentScene ||
					::scene::ID::SINGLE_PLAY != mCurrentScene->currentScene() )
				{
					// NOTE: nextScene%10은 시퀀스 내부 enum에 대응합니다.
					mCurrentScene =
						std::make_unique<::scene::inPlay::InPlay>(*mWindow, (::scene::inPlay::ID)((int)nextScene%10));
				}
				// 이미 싱글 플레이 시퀀스에 있을 경우,
				else
				{
					mCurrentScene->setScene( (uint8_t)nextScene%10, *mWindow );
				}
			}
#ifdef _DEBUG
			else
			{
				__debugbreak( );
			}
#endif
			break;
	}
}

#ifdef _DEV
void ::scene::SceneManager::chscnto( const std::string_view& args )
{
	if ( args[0] < '0' || '9' < args[0] )
	{
		// Exception
		gService()->console().printFailure( FailureLevel::WARNING, "Invalid Scene ID" );
		return;
	}
	const ::scene::ID nextScene = (::scene::ID)std::atoi( args.data() );
	// Exception
	if ( nextScene == mCurrentScene->currentScene() )
	{
		gService()->console().print( "We are already where you want to go." );
		return;
	}
	setScene( nextScene );
}

void ::scene::SceneManager::refresh( const std::string_view& )
{
	mCurrentScene->loadResources( *mWindow );
}
#endif