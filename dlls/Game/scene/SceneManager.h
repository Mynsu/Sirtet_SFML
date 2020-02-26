#pragma once
#include "IScene.h"

namespace scene
{
	class SceneManager
	{
	private:
		// 둘 이상의 인스턴스를 만들 수 없습니다.
		static bool IsInstantiated;
	public:
		// Needs init(...) to be called.
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
		// .exe로부터 얻은 포인터를 쥡니다.
		void init( sf::RenderWindow& window );

		void update( std::vector<sf::Event>& eventQueue )
		{
			const ::scene::ID nextSceneID = mCurrentScene->update(eventQueue);
			if ( ::scene::ID::AS_IS != nextSceneID )
			{
				setScene( nextSceneID );
			}
		}
		void draw( )
		{
			mCurrentScene->draw( *mWindow );
		}
	private:
		void setScene( const ::scene::ID nextSceneID );
#ifdef _DEV
		void chscnto( const std::string_view& args );
		void refresh( const std::string_view& );
#endif
		sf::RenderWindow* mWindow;
		std::unique_ptr<::scene::IScene> mCurrentScene;
	};
}