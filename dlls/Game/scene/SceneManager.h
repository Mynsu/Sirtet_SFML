#pragma once
#pragma hdrstop
#include <Lib/precompiled.h>
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
		inline SceneManager( )
			: mWindow( nullptr ), mSetScene( std::bind( &::scene::SceneManager::setScene, this, std::placeholders::_1 ) )
		{
			ASSERT_FALSE( IsInstantiated );
			IsInstantiated = true;
		}
		inline ~SceneManager( )
		{
			mWindow = nullptr;
			IsInstantiated = false;
		}
		SceneManager( const SceneManager& ) = delete;
		void operator=( const SceneManager& ) = delete;
		void lastInit( sf::RenderWindow* const window );

		inline void update( )
		{
			mCurrentScene->update( );
		}
		inline void draw( )
		{
			mCurrentScene->draw( );
		}
	private:
		void setScene( const ::scene::ID nextSceneID );
#ifdef _DEV
		// Pseudo-unnamed function
		void _2436549370( const std::string_view& args );
		// Pseudo-unnamed function
		void _495146883( const std::string_view& );
#endif
		sf::RenderWindow* mWindow;
		const SetScene_t mSetScene;
		std::unique_ptr< ::scene::IScene > mCurrentScene;
	};
}