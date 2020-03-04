#pragma once
#include "../IScene.h"
#include "IScene.h"

namespace scene::inPlay
{
	class InPlay final : public ::scene::IScene
	{
	private:
		static bool IsInstantiated;
	public:
		InPlay( sf::RenderWindow& window,
			   const ::scene::inPlay::ID initScene = ::scene::inPlay::ID::READY );
		~InPlay( );

		void loadResources( const sf::RenderWindow& window ) override;
		::scene::ID update( std::vector<sf::Event>& eventQueue ) override;
		void draw( sf::RenderWindow& window ) override;
		::scene::ID currentScene( ) const override;
		void setScene( const uint8_t sceneID ) override
		{
			setScene( (::scene::inPlay::ID)sceneID );
		}
	private:
		void setScene( const ::scene::inPlay::ID nextInPlaySceneID );
		uint16_t mFPS_;
		// NOTE: �б� ���� ��� �Լ� update(...)�� ���ڷ� �ѱ����
		// �������̵��ϰ� �ִ� ���� ���� �Լ����� �ٲ���ؼ�
		// �����ڷ� ���۷����� �޾� ��� �ֽ��ϴ�.
		sf::RenderWindow& mWindow_;
		std::unique_ptr<::scene::inPlay::IScene> mCurrentScene;
		// ���ÿ� ���� ���� ��� �ý��ϴ�.
		std::unique_ptr<::scene::inPlay::IScene> mOverlappedScene;
		sf::RectangleShape mBackground;
	};
}