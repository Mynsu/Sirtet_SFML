#include "../pch.h"
#include "TextInputBox.h"

bool ui::TextInputBox::processEvent( std::vector<sf::Event>& eventQueue )
{
	bool remainsSomethingToProcessOutside = false;
	for ( auto it = eventQueue.cbegin(); eventQueue.cend() != it; )
	{
		if ( sf::Event::TextEntered == it->type )
		{
			if ( uint32_t input = (uint32_t)it->text.unicode;
				input > 0x1f && input < 0x7f )
			{
				mInputTextFieldString += (char)input;
				mTextFieldToInput.setString( mInputTextFieldString + '_' );
			}
			it = eventQueue.erase(it);
		}
		else if ( sf::Event::KeyPressed == it->type )
		{
			switch ( it->key.code )
			{
				case sf::Keyboard::Escape:
					mIsActive = false;
					break;
				case sf::Keyboard::Enter:
					mIsActive = false;
					remainsSomethingToProcessOutside = true;
					break;
				case sf::Keyboard::Backspace:
					if ( false == mInputTextFieldString.empty() )
					{
						mInputTextFieldString.pop_back( );
						mTextFieldToInput.setString( mInputTextFieldString + '_' );
					}
					break;
				default:
					break;
			}
			it = eventQueue.erase(it);
		}
		else if ( sf::Event::MouseButtonPressed == it->type )
		{
			it = eventQueue.erase(it);
		}
		else
		{
			++it;
		}
	}
	return remainsSomethingToProcessOutside;
}
