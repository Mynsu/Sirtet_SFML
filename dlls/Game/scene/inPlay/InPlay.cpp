#include "../../pch.h"
#include "InPlay.h"
#include "../../ServiceLocatorMirror.h"
#include "../VaultKeyList.h"
#include "Ready.h"
#include "Playing.h"
#include "GameOver.h"
#include "Assertion.h"

bool ::scene::inPlay::InPlay::IsInstantiated = false;

::scene::inPlay::InPlay::InPlay( sf::RenderWindow& window )
	: mWindow_( window )
{
	ASSERT_FALSE( IsInstantiated );

	mFPS_ = (uint32_t)gService( )->vault( )[HK_FORE_FPS];
	setScene( ::scene::inPlay::ID::READY );
	loadResources( );

	IsInstantiated = true;
}

::scene::inPlay::InPlay::~InPlay( )
{
	IsInstantiated = false;
}

void scene::inPlay::InPlay::loadResources( )
{
	mBackground.setSize( sf::Vector2f(mWindow_.getSize()) );
	mCurrentScene->loadResources( );
}

::scene::ID scene::inPlay::InPlay::update( std::list< sf::Event >& eventQueue )
{
	::scene::ID retVal = ::scene::ID::AS_IS;
	const ::scene::inPlay::ID nextScene = mCurrentScene->update( eventQueue );
	if ( ::scene::inPlay::ID::AS_IS < nextScene )
	{
		setScene( nextScene );
	}
	else if ( ::scene::inPlay::ID::EXIT == nextScene )
	{
		retVal = ::scene::ID::MAIN_MENU;
	}

	if ( nullptr != mOverlappedScene )
	{
		const ::scene::inPlay::ID nextScene = mOverlappedScene->update( eventQueue );
		if ( ::scene::inPlay::ID::UNDO == nextScene )
		{
			mOverlappedScene.reset( );
		}
		else if ( ::scene::inPlay::ID::EXIT == nextScene )
		{
			retVal = ::scene::ID::MAIN_MENU;
		}
	}

	return retVal;
}

void ::scene::inPlay::InPlay::draw( )
{
	mCurrentScene->draw( );
	if ( nullptr != mOverlappedScene )
	{
		mOverlappedScene->draw( );
	}
}

#ifdef _DEV
::scene::ID scene::inPlay::InPlay::currentScene( ) const
{
	return ::scene::ID::SINGLE_PLAY;
}
#endif

void scene::inPlay::InPlay::setScene( const ::scene::inPlay::ID nextInPlaySceneID )
{
	switch ( nextInPlaySceneID )
	{
		case ::scene::inPlay::ID::READY:
			mCurrentScene = std::make_unique< ::scene::inPlay::Ready >( mWindow_, mBackground );
			break;
		case ::scene::inPlay::ID::PLAYING:
			mCurrentScene = std::make_unique< ::scene::inPlay::Playing >( mWindow_, mBackground, mOverlappedScene );
			break;
		case ::scene::inPlay::ID::GAME_OVER:
			mCurrentScene = std::make_unique< ::scene::inPlay::GameOver >( mWindow_, mBackground, mOverlappedScene );
			break;
		case ::scene::inPlay::ID::ASSERTION:
			mOverlappedScene = std::make_unique< ::scene::inPlay::Assertion >( mWindow_ );
			break;
	}
}