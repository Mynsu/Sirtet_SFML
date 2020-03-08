#pragma once
#include "IScene.h"

namespace scene
{
	class SceneManager
	{
	private:
		// �� �̻��� �ν��Ͻ��� ���� �� �����ϴ�.
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
		// .exe�κ��� ���� �����͸� ��ϴ�.
		void init( sf::RenderWindow& window );

		void update( std::vector<sf::Event>& eventQueue )
		{
			// NOTE: �� �����Ӹ��� ȣ��Ǵ� �Լ��� ���ϸ� ���̱� ���� nullptr üũ �����߽��ϴ�.
			const ::scene::ID nextSceneID = mCurrentScene->update(eventQueue);
			if ( ::scene::ID::AS_IS != nextSceneID )
			{
				setScene( nextSceneID );
			}
		}
		void draw( )
		{
			// NOTE: �� �����Ӹ��� ȣ��Ǵ� �Լ��� ���ϸ� ���̱� ���� nullptr üũ �����߽��ϴ�.
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