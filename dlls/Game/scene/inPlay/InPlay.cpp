#include "InPlay.h"
#include "Ready.h"
#include "GameOver.h"
#include "../../ServiceLocatorMirror.h"

bool ::scene::inPlay::InPlay::IsInstantiated = false;

::scene::inPlay::InPlay::InPlay( sf::RenderWindow& window,
								  const SetScene_t& setScene )
	: mFrameCount( 0u ),
	mWindow_( window ), mSetScene_( setScene )
{
	ASSERT_FALSE( IsInstantiated );

	mCurrentScene = std::make_unique< ::scene::inPlay::Ready >( mWindow_, mBackgroundRect );
	loadResources( );

	constexpr HashedKey HK_FORE_FPS = ::util::hash::Digest( "foreFPS" );
	mFPS = static_cast<uint32_t>(::ServiceLocatorMirror::Vault( )[ HK_FORE_FPS ]);

	IsInstantiated = true;
}

::scene::inPlay::InPlay::~InPlay( )
{
	IsInstantiated = false;
}

void scene::inPlay::InPlay::loadResources( )
{
	mBackgroundRect.setSize( sf::Vector2f( mWindow_.getSize( ) ) );
	mCurrentScene->loadResources( );
}

void ::scene::inPlay::InPlay::update( std::queue< sf::Event >& eventQueue )
{
	::scene::inPlay::IScene* nextScene = nullptr;
	mCurrentScene->update( &nextScene, eventQueue ); //TODO: 대신 함수 안에서는 강요 안 하잖아
	if ( nullptr != nextScene && mCurrentScene.get( ) != nextScene )
	{
		mCurrentScene.reset( nextScene );
	}
	nextScene = nullptr;

	if ( typeid(::scene::inPlay::GameOver) == typeid(*mCurrentScene) )
	{
		++mFrameCount;
	}

	if ( mFPS*5 == mFrameCount )
	{
		mSetScene_( ::scene::ID::MAIN_MENU );
	}
}

void ::scene::inPlay::InPlay::draw( )
{
	mCurrentScene->draw( );
}

::scene::ID scene::inPlay::InPlay::currentScene( ) const
{
	return ::scene::ID::SINGLE_PLAY; //TODO: 멀티일수도
}
