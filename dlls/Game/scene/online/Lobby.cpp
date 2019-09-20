#include "../../pch.h"
#include "Lobby.h"

bool ::scene::online::Lobby::IsInstantiated = false;

scene::online::Lobby::Lobby( sf::RenderWindow& window, const SetScene_t& setScene )
	: mWindow( window ), mSetScene( setScene )
{
	IsInstantiated = true;
}

scene::online::Lobby::~Lobby( )
{
	IsInstantiated = false;
}

void scene::online::Lobby::loadResources( )
{
}

void scene::online::Lobby::update( std::list<sf::Event>& eventQueue )
{
}

void scene::online::Lobby::draw( )
{
}

::scene::ID scene::online::Lobby::currentScene( ) const
{
	return ::scene::ID( );
}
