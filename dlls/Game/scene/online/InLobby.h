#pragma once
#include "IScene.h"
#include "../../ui/TextInputBox.h"

namespace scene::online
{
	class Online;

	class InLobby final : public ::scene::online::IScene
	{
	private:
		// 둘 이상의 인스턴스를 만들 수 없습니다.
		static bool IsInstantiated;
	public:
		InLobby( const sf::RenderWindow& window, ::scene::online::Online& net );
		~InLobby( );

		void loadResources( const sf::RenderWindow& window ) override;
		::scene::online::ID update( std::vector<sf::Event>& eventQueue,
								   ::scene::online::Online& net,
								   const sf::RenderWindow& window ) override;
		void draw( sf::RenderWindow& window ) override;
	private:
		struct User
		{
		public:
			User( const std::string& nickname, const sf::Font& font )
				: destinationIndex( 0 ), textFieldNickname( nickname, font )
			{ }
			User( const User& ) = delete;
			User( User&& ) = delete;
			void operator=( const User& ) = delete;
			uint8_t destinationIndex;
			sf::Text textFieldNickname;
		};
		enum class SoundIndex
		{
			SELECTION,
			NULL_MAX,
		};
		void createRoom( );
		void _createRoom( const std::string_view& );
		void joinRoom( const std::string_view& arg );
		struct
		{
			uint16_t nicknameFontSize;
			uint32_t nicknameColor;
			float totalDistanceUsersBoxToRoom, remainingDistanceUsersBoxToRoom,
				usersBoxAnimationSpeed,
				usersBoxOutlineThickness, roomOutlineThickness;
			sf::Color usersBoxColor, roomColor,
				usersBoxOutlineColor, roomOutlineColor;
			sf::Vector2f centerPosition;
			math::Vector<2> accelerationUsersBoxLeftTop0, usersBoxPosition,
				relativeAccelerationUserBoxRightBottom0, usersBoxSize,
				accelerationUsersBoxLeftTopToRoom, roomPosition,
				relativeAccelerationUsersBoxRightBottomToRoom, roomSize;
			std::vector<sf::Vector2f> movingPoints;
		} mDrawingInfo;
		bool mIsReceiving, mIsLoading;
		int8_t mRoomEntranceCase;
		uint8_t mIndexForGuideText;
		uint16_t mFrameCountUserListUpdateInterval, mFrameCountRequestDelay;
		MouseEvent mLatestMouseEvent;
		::scene::online::Online& mNet;
		std::vector<std::string> mGuideTexts;
		std::string mSoundPaths[(int)SoundIndex::NULL_MAX];
		::ui::TextInputBox mTextInputBox;
		std::unordered_map<std::string, User> mUserList;
		sf::Font mFont;
		sf::Text mTextLabelForGuide;
		sf::RectangleShape mBackground;
		sf::RectangleShape mUsersBox;
	};
}