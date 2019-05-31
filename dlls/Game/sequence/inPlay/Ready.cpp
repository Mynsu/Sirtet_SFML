#include "Ready.h"

sequence::inPlay::Ready::Ready( sf::RenderWindow& window, ::sequence::inPlay::Seq* const nextInPlaySequence )
	: ::sequence::inPlay::ISequence( window, nextInPlaySequence )
{
}
