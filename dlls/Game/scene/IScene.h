////
// ���� �ٱ� ����(��Ʈ��, ���θ޴�, �̱� �÷��� ������, �¶��� ��Ʋ ������)�� �������̽��Դϴ�.
// ::scene::inPlay::IScene.h, ::scene::online::IScene.h�� �ٸ��ϴ�.
////

#pragma once
#include <list>
#include <functional>
#include <SFML/Window.hpp>

namespace scene
{
	enum class ID
	{
		AS_IS = -1,
		INTRO = 0,
		MAIN_MENU = 1,
		SINGLE_PLAY = 2,
		ONLINE_BATTLE = 3,
	};

	class IScene
	{
	public:
		IScene( const IScene& ) = delete;
		void operator=( const IScene& ) = delete;
		virtual ~IScene( ) = default;

		virtual void loadResources( const sf::RenderWindow& window ) = 0;
		virtual ::scene::ID update( std::vector<sf::Event>& eventQueue ) = 0;
		virtual void draw( sf::RenderWindow& window ) = 0;
		// NOTE: ��� RTTI�� ���� �ʾҽ��ϴ�.
		virtual ::scene::ID currentScene( ) const = 0;
		virtual void setScene( const uint8_t sceneID ) = 0;

		// NOTE: ���ü��� protected�� �����Ͽ�
		// �������̽��� �ν��Ͻ��� ��üconcrete Ŭ������ �Ͻ������� ��������� �� ����
		// �ٸ� ������δ� ���� �� ������ �߽��ϴ�.
	protected:
		IScene( ) = default;
	};
}
