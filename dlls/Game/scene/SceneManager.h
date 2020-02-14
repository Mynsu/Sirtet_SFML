#pragma once
#include "IScene.h"

namespace scene
{
	class SceneManager
	{
	private:
		// Only a single instance for a type can live at a time, but shouldn't be accessible globally.
		// That's the difference from the class filled with static functions, or on singleton pattern and the like.
		static bool IsInstantiated;
	public:
		// NOTE: Called in compile-time, thus another initialization should be done.
		// 'lastInit(...)' will be called in 'GetGameAPI(...)' in 'Game.cpp.' in runtime.
		SceneManager( )
			: mWindow( nullptr )
		{
			ASSERT_TRUE( false == IsInstantiated );
			IsInstantiated = true;
		}
		virtual ~SceneManager( )
		{
			mWindow = nullptr;
			IsInstantiated = false;
		}
		void lastInit( sf::RenderWindow* const window );

		void update( std::vector<sf::Event>& eventQueue )
		{
			const ::scene::ID nextSceneID = mCurrentScene->update( eventQueue );
			if ( ::scene::ID::AS_IS != nextSceneID )
			{
				setScene( nextSceneID );
			}
		}
		void draw( )
		{
			mCurrentScene->draw( );
		}
	private:
		void setScene( const ::scene::ID nextSceneID );
#ifdef _DEV
		void chscnto( const std::string_view& args );
		void refresh( const std::string_view& );
#endif
		sf::RenderWindow* mWindow;
		std::unique_ptr< ::scene::IScene > mCurrentScene;
	};
}