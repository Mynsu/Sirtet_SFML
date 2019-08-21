#include "InPlay.h"
#include "Ready.h"

bool ::scene::inPlay::InPlay::IsInstantiated = false;

::scene::inPlay::InPlay::InPlay( sf::RenderWindow& window,
								  const SetScene_t& setScene )
	: mWindow( window ),
	mSetScene( setScene )
{
	ASSERT_FALSE( IsInstantiated );

	mCurrentScene = std::make_unique< ::scene::inPlay::Ready >( mWindow, mBackgroundRect );
	loadResources( );

	IsInstantiated = true;
}

::scene::inPlay::InPlay::~InPlay( )
{
	IsInstantiated = false;
}

void scene::inPlay::InPlay::loadResources( )
{
	mBackgroundRect.setSize( sf::Vector2f( mWindow.getSize( ) ) );
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
}

void ::scene::inPlay::InPlay::draw( )
{
	mCurrentScene->draw( );
}

::scene::ID scene::inPlay::InPlay::currentScene( ) const
{
	return ::scene::ID::SINGLE_PLAY; //TODO: 멀티일수도
}
