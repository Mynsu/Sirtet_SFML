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
		InLobby( sf::RenderWindow& window, ::scene::online::Online& net );
		~InLobby( );

		void loadResources( ) override;
		::scene::online::ID update( std::vector<sf::Event>& eventQueue ) override;
		void draw( ) override;
	private:
		enum class AudioIndex
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
			uint32_t nicknameFontSize, nicknameColor;
			float totalDistanceUsersBoxToRoom, remainingDistanceUsersBoxToRoom,
				usersBoxAnimationSpeed,
				usersBoxOutlineThickness, roomOutlineThickness;
			sf::Color usersBoxColor, roomColor,
				usersBoxOutlineColor, roomOutlineColor;
			math::Vector<2> accelerationUsersBoxLeftTop0, usersBoxPosition,
				relativeAccelerationUserBoxRightBottom0, usersBoxSize,
				accelerationUsersBoxLeftTopToRoom, roomPosition,
				relativeAccelerationUsersBoxRightBottomToRoom, roomSize;
			std::vector<sf::Vector2f> movingPoints;
		} mDrawingInfo;
		bool mIsReceiving, mHasJoined, mIsLoading;
		int8_t mEnteringRoom;
		uint8_t mGuideTextIndex;
		uint32_t mFrameCount_update, mFrameCount_requestDelay;
		sf::RenderWindow& mWindow_;
		::scene::online::Online& mNet;
		MouseEvent mLatestMouseEvent;
		std::vector<std::string> mGuideTexts;
		std::string mAudioList[(int)AudioIndex::NULL_MAX];
		::ui::TextInputBox mTextInputBox;
		// NOTE: std::pair<sf::Text, target point to move>.
		std::unordered_map< std::string, std::pair<sf::Text, uint8_t> > mUserList;
		sf::Font mFont;
		sf::Text mGuideTextLabel;
		sf::RectangleShape mBackground;
		sf::RectangleShape mUsersBox;
	};
}