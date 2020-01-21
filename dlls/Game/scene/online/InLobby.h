#pragma once
#include "IScene.h"
#include "../../ui/TextInputBox.h"

namespace scene::online
{
	class Online;
}

namespace scene::online
{
	class InLobby final : public ::scene::online::IScene
	{
	public:
		InLobby( ) = delete;
		InLobby( sf::RenderWindow& window, ::scene::online::Online& net );
		~InLobby( );

		void loadResources( ) override;
		::scene::online::ID update( std::list<sf::Event>& eventQueue ) override;
		void draw( ) override;
	private:
		void createRoom( );
		void _createRoom( const std::string_view& );
		void joinRoom( const std::string_view& arg );
		static bool IsInstantiated;
		bool mIsReceiving, mHasJoined;
		uint8_t mGuideTextIndex;
		uint32_t mFrameCount_update, mFrameCount_requestDelay, enteringRoom;
		math::Vector<2> mAcceleration_, mDestination_;
		sf::RenderWindow& mWindow_;
		::scene::online::Online& mNet;
		std::vector<sf::Vector2f> mMovingPoints;
		std::vector<std::string> mGuideTexts;
		::ui::TextInputBox mTextInputBox;
		// NOTE: std::pair<sf::Text, target point to move>.
		std::unordered_map< std::string, std::pair<sf::Text, uint8_t> > mUserList;
		sf::Font mFont_;
		sf::Text mGuideTextLabel;
		sf::RectangleShape mBackground;
		sf::RectangleShape mUserNicknamesBox;
	};
}