#pragma once
#pragma hdrstop
#include <Lib/precompiled.h>
#include "IScene.h"

namespace scene::inPlay
{
	class GameOver : public ::scene::inPlay::IScene
	{
	public:
		GameOver( ) = delete;
		GameOver( sf::RenderWindow& window, sf::Drawable& shapeOrSprite );
		~GameOver( ) = default;

		void loadResources( ) override;
		int8_t update( ::scene::inPlay::IScene** const, std::vector< sf::Event >& ) override
		{
			int8_t retVal = 0;

			// When mFade reaches the target,
			if ( TARGET_ALPHA == mFade )
			{
				// Frame counting starts.
				++mFrameCount;
				// 5 seconds after,
				if ( 5*mFPS == mFrameCount )
				{
					retVal = 1;
				}
			}

			return retVal;
		}
		void draw( ) override;
	private:
		const uint8_t TARGET_ALPHA;
		uint32_t mFade;
		uint32_t mFPS;
		uint32_t mFrameCount;
		sf::RenderWindow& mWindow_;
		sf::RectangleShape& mBackgroundRect_;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}