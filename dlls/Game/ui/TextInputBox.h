#pragma once

namespace ui
{
	class TextInputBox
	{
	public:
		TextInputBox( const sf::RenderWindow& window )
			: mIsActive( false )
		{
			mBackground.setSize( sf::Vector2f(window.getSize()) );
			mSubWindow.setSize( sf::Vector2f(200, 100) );
		}
		virtual ~TextInputBox( ) = default;

		void draw( sf::RenderWindow& window )
		{
			window.draw( mBackground );
			window.draw( mSubWindow );
			window.draw( mTextLabelForTitle );
			window.draw( mTextFieldToInput );
		}
		bool processEvent( std::vector<sf::Event>& eventQueue );
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
		const std::string& inputString( )
		{
			return mInputTextFieldString;
		}
		bool loadFont( std::string& fontPath )
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
		void setPosition( const sf::Vector2f position )
		{
			mSubWindow.setPosition( position );
		}
		void setSize( const sf::Vector2f size )
		{
			mSubWindow.setSize( size );
		}
		void setTitleDimension( const sf::Vector2f relativePosition, 
							   const uint16_t fontSize )
		{
			mTextLabelForTitle.setPosition( mSubWindow.getPosition() + relativePosition );
			mTextLabelForTitle.setCharacterSize( fontSize );
		}
		void setInputTextFieldDimension( const sf::Vector2f relativePosition,
										const uint16_t fontSize )
		{
			mTextFieldToInput.setPosition( mSubWindow.getPosition() + relativePosition );
			mTextFieldToInput.setCharacterSize( fontSize );
		}
		void setBackgroundColor( const sf::Color color )
		{
			mBackground.setFillColor( color );
		}
		void setColor( const sf::Color color )
		{
			mSubWindow.setFillColor( color );
		}
		void setTitleColor( const sf::Color color )
		{
			mTextLabelForTitle.setFillColor( color );
		}
		void setInputTextFieldColor( const sf::Color color )
		{
			mTextFieldToInput.setFillColor( color );
		}
	private:
		bool mIsActive;
		std::string mInputTextFieldString;
		sf::Font mFont;
		sf::Text mTextLabelForTitle, mTextFieldToInput;
		sf::RectangleShape mSubWindow, mBackground;
	};
}