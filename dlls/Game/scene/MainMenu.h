#pragma once
#include "IScene.h"

namespace scene
{
	class MainMenu final : public ::scene::IScene
	{
	public:
		MainMenu( sf::RenderWindow& window );
		~MainMenu( );

		void loadResources( sf::RenderWindow& window ) override;
		::scene::ID update( std::vector<sf::Event>& eventQueue ) override;
		void draw( sf::RenderWindow& window ) override;
		::scene::ID currentScene( ) const override;
		void setScene( const uint8_t sceneID ) override
		{}
	private:
		enum class SoundIndex
		{
			BGM,
			ON_SELECTION,
			NONE_MAX,
		};
		// Only a single instance for a type can live at a time, but shouldn't be accessible globally.
		// That's the difference from the class filled with static functions, or on singleton pattern and the like.
		static bool IsInstantiated;
		bool mIsCursorOnButton;
		::scene::ID mNextSceneID;
		std::string mSoundPaths[(int)SoundIndex::NONE_MAX];
		struct
		{
			sf::Vector2i logoSourcePosition, logoClipSize,
				buttonSingleSourcePosition, buttonSingleClipSize,
				buttonOnlineSourcePosition, buttonOnlineClipSize;
			sf::Vector2f logoDestinationPosition,
				buttonSinglePosition,
				buttonOnlinePosition;
		} mDrawingInfo;
		sf::Font mFont;
		sf::Text mCopyright;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}