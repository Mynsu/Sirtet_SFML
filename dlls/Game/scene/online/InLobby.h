#pragma once
#include <Lib/Common.h>
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
		InLobby( sf::RenderWindow& window, ::scene::online::Online& net );
		~InLobby( );

		void loadResources( sf::RenderWindow& window ) override;
		::scene::online::ID update( std::vector<sf::Event>& eventQueue,
								   ::scene::online::Online& net,
								   sf::RenderWindow& window ) override;
		void draw( sf::RenderWindow& window ) override;
	private:
		enum class SoundIndex
		{
			ON_SELECTION,
			NULL_MAX,
		};
		void createRoom( );
		void _createRoom( const std::string_view& );
		void joinRoom( const std::string_view& arg );
		static bool IsInstantiated;
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
		bool mIsReceiving, mHasJoined, mIsLoading;
		int8_t mEnteringRoom;
		uint8_t mIndexForGuideText;
		uint16_t mFrameCountUserListUpdateInterval, mFrameCountRequestDelay;
		MouseEvent mLatestMouseEvent;
		::scene::online::Online& mNet;
		std::vector<std::string> mGuideTexts;
		std::string mSoundPaths[(int)SoundIndex::NULL_MAX];
		::ui::TextInputBox mTextInputBox;
		// NOTE: std::pair<sf::Text, target point to move>.
		std::unordered_map< std::string, std::pair<sf::Text, uint8_t> > mUserList;
		sf::Font mFont;
		sf::Text mTextLabelForGuide;
		sf::RectangleShape mBackground;
		sf::RectangleShape mUsersBox;
	};
}