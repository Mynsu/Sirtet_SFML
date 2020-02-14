#include "../pch.h"
#include "SceneManager.h"
#include <Lib/ScriptLoader.h>
#include <Lib/VaultKeyList.h>
#include "../ServiceLocatorMirror.h"
#include "CommandList.h"
#include "Intro.h"
#include "MainMenu.h"
#include "inPlay/InPlay.h"
#include "online/Online.h"

bool ::scene::SceneManager::IsInstantiated = false;

void ::scene::SceneManager::lastInit( sf::RenderWindow* const window )
{
	mWindow = window;

#ifdef _DEV
	gService()->console().addCommand( CMD_CHANGE_SCENE, std::bind(&SceneManager::chscnto, this, std::placeholders::_1) );
	gService()->console().addCommand( CMD_RELOAD, std::bind(&SceneManager::refresh, this, std::placeholders::_1) );

	::scene::ID startScene = ::scene::ID::INTRO;
	const std::string scriptPathNName( "Scripts/_OnlyDuringDev.lua" );
	const std::string varName( "StartScene" );
	const auto result = ::util::script::LoadFromScript(scriptPathNName, varName);
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
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK, varName, scriptPathNName );
		}
	}
	setScene( startScene );
#else
	// NOTE: equals setScene( ::scene::ID::INTRO ).
	mCurrentScene = std::make_unique<::scene::Intro>( *window, mSetScene_ );
#endif
}

void ::scene::SceneManager::setScene( const ::scene::ID nextScene )
{
	ASSERT_NOT_NULL( mWindow );

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
#ifdef _DEBUG
			__debugbreak( );
#else
			__assume( 0 );
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
	// Exception: When the current scene id equals with the next scene id,
	if ( nextScene == mCurrentScene->currentScene() )
	{
		gService()->console().print( "We are already where you want to go." );
		return;
	}
	setScene( nextScene );
}

void ::scene::SceneManager::refresh( const std::string_view& )
{
	mWindow->clear( );
	mCurrentScene->loadResources( );
}
#endif