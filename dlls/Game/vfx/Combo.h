#pragma once

namespace vfx
{
	class Combo
	{
	public:
		Combo( ) = default;
		Combo( const Combo& ) = delete;
		void operator=( const Combo& ) = delete;
		Combo( Combo&& ) = delete;
		virtual ~Combo( ) = default;

		bool loadResources( std::string& textureFileName )
		{
			if ( false == mTexture.loadFromFile( textureFileName ) )
			{
				return false;
			}
			mSprite.setTexture( mTexture );
			return true;
		}
		void setOrigin( const sf::Vector2f origin, const float cellSize, const sf::Vector2i effectWidthHeight )
		{
			ASSERT_TRUE( 0.f <= origin.x && 0.f <= origin.y &&
						0.f < cellSize &&
						0 < effectWidthHeight.x && 0 < effectWidthHeight.y );

			mSprite.setOrigin( sf::Vector2f(effectWidthHeight)*0.5f );
			const sf::Vector2f localPos( ::model::stage::GRID_WIDTH, ::model::stage::GRID_HEIGHT );
			mSprite.setPosition( origin + localPos*cellSize*0.5f );
			mSpriteDimension = effectWidthHeight;
		}
		void draw( sf::RenderWindow& window, const uint8_t lineCleared )
		{
			mSprite.setTextureRect( sf::IntRect( 0, (lineCleared-1)*mSpriteDimension.y, mSpriteDimension.x, mSpriteDimension.y) );
			window.draw( mSprite );
		}
	private:
		sf::Vector2i mSpriteDimension;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}