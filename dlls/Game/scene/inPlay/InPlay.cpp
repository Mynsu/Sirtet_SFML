#include "../../pch.h"
#include "InPlay.h"
#include "../../ServiceLocatorMirror.h"
#include "Ready.h"
#include "Playing.h"
#include "GameOver.h"
#include "Assertion.h"
#include "AllClear.h"

bool ::scene::inPlay::InPlay::IsInstantiated = false;

::scene::inPlay::InPlay::InPlay( const sf::RenderWindow& window,
								const ::scene::inPlay::ID initScene )
{
	ASSERT_TRUE( false == IsInstantiated );

	auto& vault = gService()->vault();
	const auto it = vault.find(HK_FORE_FPS);
	ASSERT_TRUE( vault.end() != it );
	mFPS_ = (uint16_t)it->second;
	setScene( initScene, window );
	loadResources( window );

	IsInstantiated = true;
}

::scene::inPlay::InPlay::~InPlay( )
{
	IsInstantiated = false;
}

void scene::inPlay::InPlay::loadResources( const sf::RenderWindow& window )
{
	mBackground.setSize( sf::Vector2f(window.getSize()) );
	mCurrentScene->loadResources( window );
	if ( nullptr != mOverlappedScene )
	{
		mOverlappedScene->loadResources( window );
	}
}

::scene::ID scene::inPlay::InPlay::update( std::vector<sf::Event>& eventQueue,
										  const sf::RenderWindow& window )
{
	::scene::ID retVal = ::scene::ID::AS_IS;
	const ::scene::inPlay::ID nextScene = mCurrentScene->update( eventQueue );
	if ( ::scene::inPlay::ID::AS_IS < nextScene )
	{
		setScene( nextScene, window );
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

void ::scene::inPlay::InPlay::draw( sf::RenderWindow& window )
{
	mCurrentScene->draw( window );
	if ( nullptr != mOverlappedScene )
	{
		mOverlappedScene->draw( window );
	}
}

::scene::ID scene::inPlay::InPlay::currentScene( ) const
{
	return ::scene::ID::SINGLE_PLAY;
}

void scene::inPlay::InPlay::setScene( const ::scene::inPlay::ID nextInPlaySceneID,
									 const sf::RenderWindow& window )
{
	switch ( nextInPlaySceneID )
	{
		case ::scene::inPlay::ID::READY:
			mCurrentScene.reset( );
			mCurrentScene = std::make_unique<::scene::inPlay::Ready>(window, mBackground);
			break;
		case ::scene::inPlay::ID::PLAYING:
			mCurrentScene.reset( );
			mCurrentScene = std::make_unique<::scene::inPlay::Playing>(window, mBackground, mOverlappedScene);
			break;
		case ::scene::inPlay::ID::GAME_OVER:
			mCurrentScene.reset( );
			mCurrentScene = std::make_unique<::scene::inPlay::GameOver>(window, mBackground, mOverlappedScene);
			break;
		case ::scene::inPlay::ID::ALL_CLEAR:
			if ( nullptr == mCurrentScene )
			{
				mCurrentScene = std::make_unique<::scene::inPlay::Playing>(window, mBackground, mOverlappedScene);
			}
			mOverlappedScene = std::make_unique<::scene::inPlay::AllClear>(window);
			break;
		case ::scene::inPlay::ID::ASSERTION:
			if ( nullptr == mCurrentScene )
			{
				mCurrentScene = std::make_unique<::scene::inPlay::Playing>(window, mBackground, mOverlappedScene);
			}
			mOverlappedScene = std::make_unique<::scene::inPlay::Assertion>(window);
			break;
		default:
#ifdef _DEBUG
			__debugbreak( );
#else
			__assume( 0 );
#endif
	}
}