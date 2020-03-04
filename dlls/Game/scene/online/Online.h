#pragma once
#include "../IScene.h"
#include "IScene.h"

namespace scene::online
{
	class Online final : public ::scene::IScene
	{
	public:
		enum class Option
		{
			DEFAULT,
			FINDING_END_TO_BEGIN,
			RETURNING_TAG_ATTACHED,
		};

		Online( const sf::RenderWindow& window );
		~Online( );

		void loadResources( const sf::RenderWindow& window ) override;
		::scene::ID update( std::vector<sf::Event>& eventQueue ) override;
		void draw( sf::RenderWindow& window ) override;
		::scene::ID currentScene( ) const override;
		void setScene( const uint8_t sceneID ) override
		{}

		void setMyNickname( std::string& myNickname );
		const std::string& myNickname( ) const;
		HashedKey myNicknameHashed( ) const;
		////
		// 외부에서 Online.cpp의 전역변수에 접근하기 위한 함수들입니다.
		///
		void connectToQueueServer( );
		// Stops receiving, disconnects from the queue server, and connects to the main server.
		bool connectToMainServer( );
		// Disconnects and goes back to main menu.
		void disconnect( );
		void send( char* const data, const int size );
		void send( Packet& packet );
		// Ignores already received stuff and resumes receiving.
		void receive( );
		bool hasReceived( );
		std::optional<std::string> getByTag( const Tag tag,
											 const Online::Option option,
											uint16_t bodySize ) const;
		////
	private:
		static bool IsInstantiated;
		void setScene( const ::scene::online::ID nextSceneID );
		uint16_t mFPS_, mFrameCountToMainMenu;
		HashedKey mMyNicknameHashed_;
		sf::Vector2f mSpriteClipSize;
		const sf::RenderWindow& mWindow;
		std::unique_ptr<::scene::online::IScene> mCurrentScene;
		std::string mMyNickname;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
	
	inline Online::Option operator|( const Online::Option lh, const Online::Option rh )
	{
		return (Online::Option)((uint32_t)lh | (uint32_t)rh);
	}
	inline Online::Option& operator|=( Online::Option& lh, const Online::Option rh )
	{
		return lh = lh | rh;
	}
	inline bool operator&( const Online::Option lh, const Online::Option rh )
	{
		return (bool)((uint32_t)lh & (uint32_t)rh);
	}
}
