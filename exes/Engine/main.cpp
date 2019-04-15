#pragma hdrstop
#include <iostream>
#include <SFML\Graphics.hpp>
#include "Lib\Endian.h"
#include "ServiceLocator.h"

//TODO:
//std::unique_ptr< IConsole > Console;//

int main( int argc, char* argv[ ] )
{
/*
=====
Handling arguments
=====
*/
	//TODO: string_view를 써볼까?
	const std::string argHelp0( "-help" );
	const std::string argHelp1( "-h" );
	const std::string argWinSize( "-WS" );
	const std::string argFullscreen( "-FS" );
	int width = 800;
	int height = 600;
	bool isFullScreen = false;

	//TODO: 잘 되나?
	for ( int i = 1; i < argc; ++i )
	{
		const char* arg = argv[ i ];
		if ( 0 == argHelp0.compare( arg ) || 0 == argHelp1.compare( arg ) )
		{
			std::cout << "Here are case-sensitive parameters available:\n";
			std::cout << "\t" << argHelp0.data( ) << ", " << argHelp1.data( ) << ": Show parameters with their own arguments.\n";
			std::cout << "\t" << argWinSize.data( ) << " [width] [height]: Set the window size.\n";
			std::cout << "\t" << argFullscreen.data( ) << ": Excute Tetris on the fullscreen mode.\n";

			return 0;
		}
		else if ( 0 == argWinSize.compare( arg ) )
		{
			const int subArg0 = std::atoi( argv[ ++i ] );
			// Exception: When NON-number characters has been input,
			if ( 0 == subArg0 )
			{
				std::cerr << "Error: Let me know a number, not character.\n";
				return -1;
			}
			// Exception: When the argument is too low to be a WIDTH value,
			else if ( 400 >= subArg0 )
			{
				std::cerr << "Error: Too narrow width.\n";
				return -1;
			}
			width = subArg0;

			const int subArg1 = std::atoi( argv[ ++i ] );
			// Exception: When NON-number characters has been input,
			if ( 0 == subArg1 )
			{
				std::cerr << "Error: Let me know a number, not character.\n";
				return -1;
			}
			// Exception: When the argument is too low to be a HEIGHT value,
			else if ( 300 >= subArg1 )
			{
				std::cerr << "Error: Too low height.\n";
				return -1;
			}
			height = subArg1;

			// Exception: When the third sub-argument has been input,
			if ( 0 != std::atoi( argv[ i + 1 ] ) )
			{
				std::cerr << "Error: I have no idea of what the third means.\n";
				return -1;
			}
		}
		else if ( 0 == argFullscreen.compare( arg ) )
		{
			isFullScreen = true;
		}
		else
		{
			std::cerr << "Error: There is no such parameter.\n";
			return -1;
		}
	}

/*
=====
Initialization
=====
*/
	::util::endian::BindConvertFunc( );

/*
=====
Window
=====
*/
	sf::RenderWindow window( sf::VideoMode( width, height ), "Sirtet: the Classic", sf::Style::Close );
	window.setFramerateLimit( 60 );

	bool isOpen = true;
	while ( true == isOpen )
	{
		sf::Event event;
		while ( window.pollEvent( event ) )
		{
			if ( sf::Event::Closed == event.type ||
				 true == sf::Keyboard::isKeyPressed( sf::Keyboard::Escape ) )
			{
				isOpen = false;
			}
		}

		window.clear( );

/*
=====
Window - Console
=====
*/
		window.draw( ServiceLocator::console( ) );

		window.display( );
	}




/*
=====
Resource Free
=====
*/
	window.close( );
}