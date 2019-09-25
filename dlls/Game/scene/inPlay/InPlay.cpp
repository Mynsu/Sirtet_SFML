#include "../../pch.h"
#include "InPlay.h"
#include "Ready.h"
#include "GameOver.h"
#include "Assertion.h"
#include "../../ServiceLocatorMirror.h"

bool ::scene::inPlay::InPlay::IsInstantiated = false;

::scene::inPlay::InPlay::InPlay( sf::RenderWindow& window, 
								 const SetScene_t& setScene, 
								 const ::scene::ID mode )
	: mMode( mode ), mWindow_( window ), mSetScene( setScene )
{
	ASSERT_FALSE( IsInstantiated );

	switch ( mode )
	{
		case ::scene::ID::SINGLE_PLAY:
			mCurrentScene = std::make_unique< ::scene::inPlay::Ready >( mWindow_, mBackgroundRect );
			break;
		case ::scene::ID::ONLINE_BATTLE:
			///mCurrentScene = std::make_unique< ::scene::inPlay::Lobby >(  )
			break;
		default:
#ifdef _DEBUG
			__debugbreak();
#else
			__assume(0);
#endif
	}
	loadResources( );

	constexpr HashedKey HK_FORE_FPS = ::util::hash::Digest( "foreFPS", 7 );
	mFPS_ = static_cast<uint32_t>((*glpService).vault( )[ HK_FORE_FPS ]);

	IsInstantiated = true;
}

::scene::inPlay::InPlay::~InPlay( )
{
	IsInstantiated = false;
}

void scene::inPlay::InPlay::loadResources( )
{
	mBackgroundRect.setSize( sf::Vector2f(mWindow_.getSize()) );
	mCurrentScene->loadResources( );
}

void ::scene::inPlay::InPlay::update( std::list< sf::Event >& eventQueue )
{
	::scene::inPlay::IScene* nextScene = nullptr;
	if ( const int8_t bi = mCurrentScene->update( &nextScene, eventQueue ); 1 == bi )
	{
		mSetScene( ::scene::ID::MAIN_MENU );
		return;
	}
	if ( nullptr != mOverlappedScene )
	{
		// NOTE: _ignored won't be used.
		::scene::inPlay::IScene* _ignored = nullptr;
		const int8_t tri = mOverlappedScene->update( &_ignored, eventQueue );
		if ( -1 == tri )
		{
			mOverlappedScene.reset( );
		}
		else if ( 1 == tri )
		{
			mSetScene( ::scene::ID::MAIN_MENU );
			return;
		}
	}
	// When a sort of scene change has triggered in update(...),
	if ( nullptr != nextScene && mCurrentScene.get( ) != nextScene )
	{
		if ( typeid(::scene::inPlay::Assertion) == typeid(*nextScene) )
		{
			mOverlappedScene.reset( nextScene );
		}
		else
		{
			mCurrentScene.reset( nextScene );
		}
		nextScene = nullptr;
	}
}

void ::scene::inPlay::InPlay::draw( )
{
	mCurrentScene->draw( );
	if ( nullptr != mOverlappedScene )
	{
		mOverlappedScene->draw( );
	}
}

::scene::ID scene::inPlay::InPlay::currentScene( ) const
{
	return ::scene::ID::SINGLE_PLAY; //TODO: 멀티일수도
}
