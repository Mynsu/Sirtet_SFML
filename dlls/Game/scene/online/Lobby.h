#pragma once
#include "../IScene.h"

namespace scene::online
{
	class Lobby : public ::scene::IScene
	{
	public:
		Lobby( ) = delete;
		Lobby( sf::RenderWindow& window, const SetScene_t& setScene );
		~Lobby( );
	private:
		static bool IsInstantiated;
		const SetScene_t& mSetScene;
		sf::RenderWindow& mWindow;

		// IScene��(��) ���� ��ӵ�
		virtual void loadResources( ) override;
		virtual void update( std::list<sf::Event>& eventQueue ) override;
		virtual void draw( ) override;
		virtual ::scene::ID currentScene( ) const override;
	};
}