#pragma once
#include "IScene.h"

namespace scene
{
	class MainMenu final : public ::scene::IScene
	{
	private:
		enum class SoundIndex
		{
			BGM,
			SELECTION,
			NONE_MAX,
		};
		// 둘 이상의 인스턴스를 만들 수 없습니다.
		static bool IsInstantiated;
	public:
		MainMenu( const sf::RenderWindow& window );
		~MainMenu( );

		void loadResources( const sf::RenderWindow& window ) override;
		::scene::ID update( std::vector<sf::Event>& eventQueue,
						   const sf::RenderWindow& window ) override;
		void draw( sf::RenderWindow& window ) override;
		::scene::ID currentScene( ) const override;
		void setScene( const uint8_t sceneID,
					  const sf::RenderWindow& ) override
		{}
	private:
		bool mIsCursorOnButton;
		::scene::ID mNextSceneID;
		struct
		{
			sf::Vector2i logoSourcePosition, logoClipSize,
				buttonSingleSourcePosition, buttonSingleClipSize,
				buttonOnlineSourcePosition, buttonOnlineClipSize;
			sf::Vector2f logoDestinationPosition,
				buttonSinglePosition,
				buttonOnlinePosition;
		} mDrawingInfo;
		std::string mSoundPaths[(int)SoundIndex::NONE_MAX];
		sf::Font mFont;
		sf::Text mTextLabelForCopyrightNotice;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}