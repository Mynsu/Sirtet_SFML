#include "SceneManager.h"
#include <Lib/ScriptLoader.h>
#include "../ServiceLocatorMirror.h"
#include "Intro.h"
#include "MainMenu.h"
#include "inPlay/InPlay.h"

bool ::scene::SceneManager::IsInstantiated = false;

void ::scene::SceneManager::lastInit( sf::RenderWindow* const window )
{
	mWindow = window;

#ifdef _DEV
	//
	// Registering commands
	///
	constexpr HashedKey HK_COMMAND0 = ::util::hash::Digest( "chseqto" );
	ServiceLocatorMirror::Console( )->addCommand( HK_COMMAND0, std::bind( &SceneManager::_2436549370, this, std::placeholders::_1 ) );
	constexpr HashedKey HK_COMMAND1 = ::util::hash::Digest( "refresh" );
	ServiceLocatorMirror::Console( )->addCommand( HK_COMMAND1, std::bind( &SceneManager::_495146883, this, std::placeholders::_1 ) );

	//
	// Starting scene
	///
	::scene::ID startScene = ::scene::ID::INTRO;
	const std::string scriptPathNName( "Scripts/_OnlyDuringDev.lua" );
	const std::string varName( "StartScene" );
	const auto result = ::util::script::LoadFromScript( scriptPathNName, varName );
	// When there's the variable 'StartScene' in the script,
	if ( const auto it = result.find( varName ); result.cend( ) != it )
	{
		// When its type is interger which can be cast to enum type,
		if ( true == std::holds_alternative< int >( it->second ) )
		{
			startScene = static_cast< ::scene::ID >( std::get< int >( it->second ) );
		}
		// Type Check Exception
		else
		{
			ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK, varName, scriptPathNName );
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
			mCurrentScene = std::make_unique< ::scene::Intro >( *mWindow, mSetScene );
			break;
		case ::scene::ID::MAIN_MENU:
			mCurrentScene = std::make_unique< ::scene::MainMenu >( *mWindow, mSetScene );
			break;
		case ::scene::ID::SINGLE_PLAY:
			mCurrentScene = std::make_unique< ::scene::inPlay::InPlay >( *mWindow, mSetScene );
			break;
		case ::scene::ID::MULTI_PLAY:
		default:
#ifdef _DEBUG
			__debugbreak( );
#else
			__assume( 0 );
#endif
	}
}

#ifdef _DEV
void ::scene::SceneManager::_2436549370( const std::string_view& args )
{
	const ::scene::ID nextScene = static_cast< ::scene::ID >( std::atoi( args.data( ) ) );
	// Exception: When the current scene id equals with the next scene id,
	if ( nextScene == mCurrentScene->currentScene( ) )
	{
		::ServiceLocatorMirror::Console( )->print( "We are already where you want to go." );
		return;
	}
	setScene( nextScene );
}

void ::scene::SceneManager::_495146883( const std::string_view& )
{
	mWindow->clear( );
	mCurrentScene->loadResources( );
}
#endif