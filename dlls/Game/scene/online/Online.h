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
			FIND_END_TO_BEGIN,
			RETURN_TAG_ATTACHED,
		};

		Online( sf::RenderWindow& window );
		~Online( );

		void loadResources( ) override;
		::scene::ID update( std::vector<sf::Event>& eventQueue ) override;
		void draw( ) override;

		void connectToQueueServer( );
		void disconnect( );
		// Stop receiving and disconnect from the queue server, and connect to the main server.
		bool connectToMainServer( );
		void send( char* const data, const int size );
		void send( Packet& packet );
		// Ignore received stuff and resume reception.
		void receive( );
		bool hasReceived( );
		std::optional<std::string> getByTag( const Tag tag,
											 const Online::Option option,
											uint16_t bodySize ) const;
		void setMyNickname( std::string& myNickname );
		const std::string& myNickname( ) const;
		HashedKey myNicknameHashed( ) const;
#ifdef _DEV
		::scene::ID currentScene( ) const override;
#endif
	private:
		void setScene( const ::scene::online::ID nextSceneID );
		static bool IsInstantiated;
		// When 0 frame counters don't increase, while 1 or more they increase.
		// In other words, setting the value 0 to 1( or any number except 0 ) acts like a trigger.
		uint16_t mFPS_, mFrameCountToMainMenu;
		HashedKey mMyNicknameHashed_;
		sf::Vector2f mSpriteClipSize_;
		sf::RenderWindow& mWindow_;
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
