#pragma once

namespace ui
{
	class TextInputBox
	{
	public:
		TextInputBox( sf::RenderWindow& window )
			: mIsActive( false ), mWindow_( window )
		{
			mBackground.setSize( sf::Vector2f(window.getSize()) );
			mSubWindow.setSize( sf::Vector2f(200, 100) );
		}
		virtual ~TextInputBox( ) = default;

		void setBackgroundColor( const sf::Color color )
		{
			mBackground.setFillColor( color );
		}
		void setPosition( const sf::Vector2f position )
		{
			mSubWindow.setPosition( position );
		}
		void setSize( const sf::Vector2f size )
		{
			mSubWindow.setSize( size );
		}
		void setColor( const sf::Color color )
		{
			mSubWindow.setFillColor( color );
		}
		bool loadFont( const std::string& fontPathNName )
		{
			bool result = true;
			if ( false == mFont.loadFromFile(fontPathNName) )
			{
				result = false;
				return result;
			}
			mTitleLabel.setFont( mFont );
			mInputTextField.setFont( mFont );
			return result;
		}
		void setTitleDimension( const sf::Vector2f relativePosition, 
							   const uint32_t fontSize )
		{
			mTitleLabel.setPosition( mSubWindow.getPosition() + relativePosition );
			mTitleLabel.setCharacterSize( fontSize );
		}
		void setTitleColor( const sf::Color color )
		{
			mTitleLabel.setFillColor( color );
		}
		void setInputTextFieldDimension( const sf::Vector2f relativePosition,
										const uint32_t fontSize )
		{
			mInputTextField.setPosition( mSubWindow.getPosition() + relativePosition );
			mInputTextField.setCharacterSize( fontSize );
		}
		void setInputTextFieldColor( const sf::Color color )
		{
			mInputTextField.setFillColor( color );
		}
		bool isActive( ) const
		{
			return mIsActive;
		}
		void activate( const std::string& title )
		{
			mTitleLabel.setString( title );
			mInputTextFieldString.clear( );
			mInputTextField.setString( mInputTextFieldString + '_' );
			mIsActive = true;
		}
		void deactivate( )
		{
			mIsActive = false;
		}
		bool processEvent( std::vector<sf::Event>& eventQueue );
		const std::string& inputString( )
		{
			return mInputTextFieldString;
		}
		void draw( )
		{
			mWindow_.draw( mBackground );
			mWindow_.draw( mSubWindow );
			mWindow_.draw( mTitleLabel );
			mWindow_.draw( mInputTextField );
		}
	private:
		bool mIsActive;
		sf::RenderWindow& mWindow_;
		std::string mInputTextFieldString;
		sf::Font mFont;
		sf::Text mTitleLabel, mInputTextField;
		sf::RectangleShape mSubWindow, mBackground;
	};
}