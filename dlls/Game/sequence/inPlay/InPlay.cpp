#include "InPlay.h"
#include "SinglePlay.h"

sequence::inPlay::InPlay::InPlay( sf::RenderWindow& window,
							  ::sequence::Seq* nextSequence )
	: mWindow( window )
{
}

void sequence::inPlay::InPlay::update( )
{
	mCurrentSequence->update( );
}

void sequence::inPlay::InPlay::draw( )
{
	mCurrentSequence->draw( );
}
