////
// ���÷��� ����(���Ժ� ī��Ʈ�ٿ�, �̱� �÷���, ���� ����, �� Ŭ����, ������ �� �� �ٽ� ���� ��)�� �������̽��Դϴ�.
// �� ������ �̱� �÷��� ������ �ƴϰ� �����ε� �¶��� ��Ʋ ���������� ���̰� �ִٴ� �Ϳ� �����ؾ� �մϴ�.
// ::scene::IScene.h, ::scene::online::IScene.h�� �ٸ��ϴ�.
////
#pragma once
#include <list>
#include <SFML/Window.hpp>

namespace scene::inPlay
{
	enum class ID
	{
		EXIT = -3,
		UNDO = -2,
		AS_IS = -1,
		READY,
		PLAYING,
		GAME_OVER,
		ALL_CLEAR, // All-levels-clear scene.
		ASSERTION,
	};

	class IScene
	{
	public:
		IScene( const IScene& ) = delete;
		void operator=( const IScene& ) = delete;
		virtual ~IScene( ) = default;
		
		virtual void loadResources( sf::RenderWindow& window ) = 0;
		virtual ::scene::inPlay::ID update( std::vector<sf::Event>& eventQueue ) = 0;
		virtual void draw( sf::RenderWindow& window ) = 0;

		// NOTE: ���ü��� protected�� �����Ͽ�
		// �������̽��� �ν��Ͻ��� ��üconcrete Ŭ������ �Ͻ������� ��������� �� ����
		// �ٸ� ������δ� ���� �� ������ �߽��ϴ�.
	protected:
		IScene( ) = default;
	};
}