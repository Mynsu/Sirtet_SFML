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
			const sf::Vector2f winSize( mWindow_.getSize() );
			ASSERT_TRUE( position.x < winSize.x && position.y < winSize.y );
			mSubWindow.setPosition( position );
		}
		void setSize( const sf::Vector2f size )
		{
			const sf::Vector2f winSize( mWindow_.getSize() );
			ASSERT_TRUE( size.x < winSize.x && size.y < winSize.y );
			mSubWindow.setSize( size );
		}
		void setColor( const sf::Color color )
		{
			mSubWindow.setFillColor( color );
		}
		bool loadFont( const std::string& fontPath )
		{
			bool result = true;
			if ( false == mFont.loadFromFile(fontPath) )
			{
				result = false;
				return result;
			}
			mTextLabelForTitle.setFont( mFont );
			mTextFieldToInput.setFont( mFont );
			return result;
		}
		void setTitleDimension( const sf::Vector2f relativePosition, 
							   const uint16_t fontSize )
		{
			mTextLabelForTitle.setPosition( mSubWindow.getPosition() + relativePosition );
			mTextLabelForTitle.setCharacterSize( fontSize );
		}
		void setTitleColor( const sf::Color color )
		{
			mTextLabelForTitle.setFillColor( color );
		}
		void setInputTextFieldDimension( const sf::Vector2f relativePosition,
										const uint16_t fontSize )
		{
			mTextFieldToInput.setPosition( mSubWindow.getPosition() + relativePosition );
			mTextFieldToInput.setCharacterSize( fontSize );
		}
		void setInputTextFieldColor( const sf::Color color )
		{
			mTextFieldToInput.setFillColor( color );
		}
		bool isActive( ) const
		{
			return mIsActive;
		}
		void activate( const std::string& title )
		{
			mTextLabelForTitle.setString( title );
			mInputTextFieldString.clear( );
			mTextFieldToInput.setString( mInputTextFieldString + '_' );
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
			mWindow_.draw( mTextLabelForTitle );
			mWindow_.draw( mTextFieldToInput );
		}
	private:
		bool mIsActive;
		sf::RenderWindow& mWindow_;
		std::string mInputTextFieldString;
		sf::Font mFont;
		sf::Text mTextLabelForTitle, mTextFieldToInput;
		sf::RectangleShape mSubWindow, mBackground;
	};
}