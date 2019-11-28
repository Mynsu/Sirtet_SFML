#pragma once
#include "../IScene.h"
#include "IScene.h"

namespace scene::online
{
	class Online final : public ::scene::IScene
	{
	public:
		Online( ) = delete;
		Online( sf::RenderWindow& window );
		~Online( );

		void loadResources( ) override;
		::scene::ID update( std::list<sf::Event>& eventQueue ) override;
		void draw( ) override;

		bool connectToMainServer( );
		void disconnect( );
		void send( char* data, const int size );
		void send( std::string& data );
		void receive( );
		bool hasSent( );
		bool hasReceived( );
		enum class Option
		{
			NONE = 0,
			FIND_END_TO_BEGIN = 1 << 0,
			SERIALIZED = 1 << 1,
			RETURN_TAG_ATTACHED = 1 << 2,
		};
		std::optional< std::string > getByTag( const Tag tag, const Online::Option option );
		///char* const receivingBuffer( );
		void stopReceivingFromQueueServer( );
		void setNickname( std::string& nickname )
		{
			mNickname = nickname;
		}
		const std::string& nickname( ) const
		{
			return mNickname;
		}
#ifdef _DEV
		::scene::ID currentScene( ) const override;
#endif
	private:
		void setScene( const ::scene::online::ID nextSceneID );
		static bool IsInstantiated;
		// When 0 frame counters don't increase, while 1 or more they increase.
		// In other words, setting the value 0 to 1( or any number except 0 ) acts like a trigger.
		uint32_t mFPS_, mFrameCount_disconnection;
		sf::Vector2f mSpriteClipSize_;
		sf::RenderWindow& mWindow_;
		std::unique_ptr< ::scene::online::IScene > mCurrentScene;
		std::string mNickname;
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
