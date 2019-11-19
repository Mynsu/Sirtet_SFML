#pragma once

namespace vfx
{
	class Combo
	{
	public:
		Combo( ) = delete;
		inline Combo( sf::RenderWindow& window )
			: mWindow_( window )
		{
		}
		inline ~Combo( ) = default;

		inline bool loadResources( const std::string& textureFileName )
		{
			if ( false == mTexture.loadFromFile( textureFileName ) )
			{
				return false;
			}
			mSprite.setTexture( mTexture );
			return true;
		}
		inline void setOrigin( const sf::Vector2f origin, const float cellSize, const sf::Vector2i effectWidthHeight )
		{
			ASSERT_TRUE( (0<=origin.x) && (0<=origin.y) && (0<cellSize) && (0<effectWidthHeight.x) && (0<effectWidthHeight.y) );

			mSprite.setOrigin( sf::Vector2f(effectWidthHeight)*0.5f );
			const sf::Vector2f localPos( ::model::stage::GRID_WIDTH, ::model::stage::GRID_HEIGHT );
			mSprite.setPosition( origin + localPos*cellSize*0.5f );
			mSpriteDimension = effectWidthHeight;
		}
		void draw( const uint8_t lineCleared );
	private:
		sf::RenderWindow& mWindow_;
		sf::Vector2i mSpriteDimension;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}