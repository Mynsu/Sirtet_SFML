#include "../pch.h"
#include "SceneManager.h"
#include <Lib/ScriptLoader.h>
#include "../ServiceLocatorMirror.h"
#include "CommandList.h"
#include "../VaultKeyList.h"
#include "Intro.h"
#include "MainMenu.h"
#include "inPlay/InPlay.h"
#include "online/Online.h"

bool ::scene::SceneManager::IsInstantiated = false;
namespace
{
	void Exit( const std::string_view& )
	{
		gService( )->vault( )[ HK_IS_RUNNING ] = 0;
	}
}

void ::scene::SceneManager::lastInit( sf::RenderWindow* const window )
{
	mWindow = window;

	//
	// Registering commands
	///
	gService( )->console( ).addCommand( CMD_EXIT, &::Exit );
#ifdef _DEV
	gService( )->console( ).addCommand( CMD_CHANGE_SCENE, std::bind(&SceneManager::chscnto, this, std::placeholders::_1) );
	gService( )->console( ).addCommand( CMD_RELOAD, std::bind(&SceneManager::refresh, this, std::placeholders::_1) );

	//
	// Starting scene
	///
	::scene::ID startScene = ::scene::ID::INTRO;
	const char scriptPathNName[] = "Scripts/_OnlyDuringDev.lua";
	const char varName[] = "StartScene";
	const auto result = ::util::script::LoadFromScript( scriptPathNName, varName );
	// When there's the variable 'StartScene' in the script,
	if ( const auto it = result.find(varName); result.cend() != it )
	{
		// When its type is interger which can be cast to enum type,
		if ( true == std::holds_alternative<int>(it->second) )
		{
			startScene = (::scene::ID)std::get< int >( it->second );
		}
		// Type Check Exception
		else
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK, varName, scriptPathNName );
		}
	}
	/// Variable Not Found Exception
	///else { // Nothing to do }
	setScene( startScene );
#else
	// NOTE: equals setScene( ::scene::ID::INTRO ).
	mCurrentScene = std::make_unique< ::scene::Intro >( *window, mSetScene_ );
#endif
}

void ::scene::SceneManager::setScene( const ::scene::ID nextScene )
{
	ASSERT_NOT_NULL( mWindow );

	mCurrentScene.reset( nullptr );
	switch ( nextScene )
	{
		case ::scene::ID::INTRO:
			mCurrentScene = std::make_unique< ::scene::Intro >( *mWindow );
			break;
		case ::scene::ID::MAIN_MENU:
			mCurrentScene = std::make_unique< ::scene::MainMenu >( *mWindow );
			break;
		case ::scene::ID::SINGLE_PLAY:
			//TODO: inPlay를 localPlay로 개명.
			mCurrentScene = std::make_unique< ::scene::inPlay::InPlay >( *mWindow );
			break;
//TODO:싱글 플레이 하위 씬들도 여기에 추가.
		case ::scene::ID::ONLINE_BATTLE:
			mCurrentScene = std::make_unique< ::scene::online::Online >( *mWindow );
			break;
		default:
#ifdef _DEBUG
			__debugbreak( );
#else
			__assume( 0 );
#endif
	}
}

#ifdef _DEV
void ::scene::SceneManager::chscnto( const std::string_view& args )
{
	if ( args[0] < '0' || '9' < args[0] )
	{
		// Exception
		gService( )->console( ).printFailure( FailureLevel::WARNING, "Invalid Scene ID" );
		return;
	}
	const ::scene::ID nextScene = (::scene::ID)std::atoi( args.data() );
	// Exception: When the current scene id equals with the next scene id,
	if ( nextScene == mCurrentScene->currentScene() )
	{
		gService( )->console( ).print( "We are already where you want to go." );
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