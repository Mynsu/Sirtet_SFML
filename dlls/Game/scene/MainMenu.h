#pragma once
#include "IScene.h"

namespace scene
{
	class MainMenu final : public ::scene::IScene
	{
	public:
		MainMenu( sf::RenderWindow& window );
		~MainMenu( );

		void loadResources( ) override;
		::scene::ID update( std::vector<sf::Event>& eventQueue ) override;
		void draw( ) override;
#ifdef _DEV
		::scene::ID currentScene( ) const override;
#endif
	private:
		enum class AudioIndex
		{
			BGM,
			ON_SELECTION,
			NONE_MAX,
		};

		void touchButton( );

		// Only a single instance for a type can live at a time, but shouldn't be accessible globally.
		// That's the difference from the class filled with static functions, or on singleton pattern and the like.
		static bool IsInstantiated;
		bool mIsCursorOnButton;
		sf::RenderWindow& mWindow_;
		::scene::ID mNextSceneID;
		sf::Vector2f mSpriteClipSize_;
		sf::Vector2f mLogoMargin_;
		sf::Vector2f mButtonSinglePosition_, mButtonOnlinePosition_;
		std::string mAudioList[(int)AudioIndex::NONE_MAX];
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}