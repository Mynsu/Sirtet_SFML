#include "../pch.h"
#include "TextInputBox.h"

bool ui::TextInputBox::processEvent( std::vector<sf::Event>& eventQueue )
{
	bool remainsSomethingToProcessOutside = false;
	eventQueue.erase(std::remove_if(eventQueue.begin(), eventQueue.end(), [this, &remainsSomethingToProcessOutside](const sf::Event& event)->bool
									{
										bool isRemoved = false;
										if ( sf::Event::TextEntered == event.type )
										{
											if ( uint32_t input = (uint32_t)event.text.unicode;
												input > 0x1f && input < 0x7f )
											{
												mInputTextFieldString += (char)input;
												mTextFieldToInput.setString( mInputTextFieldString + '_' );
											}
											isRemoved = true;
										}
										else if ( sf::Event::KeyPressed == event.type )
										{
											switch ( event.key.code )
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
											isRemoved = true;
										}
										else if ( sf::Event::MouseButtonPressed == event.type )
										{
											isRemoved = true;
										}
										return isRemoved;
									}), eventQueue.end());
	
	return remainsSomethingToProcessOutside;
}
