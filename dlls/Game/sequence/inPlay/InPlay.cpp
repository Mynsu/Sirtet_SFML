#include "InPlay.h"
#include "Ready.h"

bool sequence::inPlay::InPlay::IsInstanciated = false;

sequence::inPlay::InPlay::InPlay( sf::RenderWindow& window,
							  ::sequence::Seq* const nextMainSequence )
	: ::sequence::ISequence( window, nextMainSequence )
{
	ASSERT_FALSE( IsInstanciated );
	*iNextMainSequence = ::sequence::Seq::NONE;
	auto& varT = ::global::VariableTable( );
	varT.emplace( 1020163455, static_cast< uint32_t >( ::sequence::inPlay::Seq::READY ) ); // nextInPlaySeq
	mNextInPlaySequence = reinterpret_cast< ::sequence::inPlay::Seq* >( &varT.find( 1020163455 )->second ); // nextInPlaySeq
	const std::string pathAndFilename( "Images/InPlay.png" );
	if ( false == iTexture.loadFromFile( pathAndFilename ) )
	{
		::global::Console( )->printError( "Failed to load " + pathAndFilename );
	}
	iSprite.setTexture( iTexture );
	IsInstanciated = true;
}

void sequence::inPlay::InPlay::update( )
{
	switch ( *mNextInPlaySequence )
	{
		case ::sequence::inPlay::Seq::READY:
			moveTo< ::sequence::inPlay::Ready >( );
			break;
		case ::sequence::inPlay::Seq::NONE:
			break;
		default:
			__assume( 0 );
			break;
	}
	mCurrentInPlaySequence->update( );
}

void sequence::inPlay::InPlay::draw( )
{
	sf::RectangleShape rect;
	rect.setSize( sf::Vector2f( iWindow.getSize( ) ) );
	const unsigned BACKGROUND_RGB = 0x29cdb500u;
	const unsigned MAX_RGB = 0xffffff00u;
	const unsigned SHADOW = 0x7fu;
	switch ( *mNextInPlaySequence )
	{
		case ::sequence::inPlay::Seq::READY:
			rect.setFillColor( sf::Color( BACKGROUND_RGB | SHADOW ) );
			iSprite.setColor( sf::Color( MAX_RGB | SHADOW ) );
			break;
		case ::sequence::inPlay::Seq::NONE:
			rect.setFillColor( sf::Color( BACKGROUND_RGB | 0xffu ) );
			iSprite.setColor( sf::Color( MAX_RGB | 0xffu ) );
			break;
		default:
			__assume( 0 );
			break;
	}
	iWindow.draw( rect );
	iSprite.setPosition( rect.getSize( ) / 3.f );
	iWindow.draw( iSprite );
	mCurrentInPlaySequence->draw( );
}
