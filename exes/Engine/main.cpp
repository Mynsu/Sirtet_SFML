#pragma hdrstop
#include <iostream>
#include <SFML\Graphics.hpp>
#include "Lib\Endian.h"
#include "ServiceLocator.h"

int main( int argc, char* argv[ ] )
{
/*
=====
Handling arguments
=====
*/
	int winWidth = 800;
	int winHeight = 600;
	uint8_t winStyle = sf::Style::Close;
	{
		const std::string_view argHelp0( "--help" );
		const std::string_view argHelp1( "--h" );
		const std::string_view argWinSize( "--WS" );
		const std::string_view argFullscreen( "--FS" );

		for ( int i = 1; i < argc; ++i )
		{
			const char* cur = argv[ i ];
			if ( 0 == argHelp0.compare( cur ) || 0 == argHelp1.compare( cur ) )
			{
				std::cout << "Here are case-sensitive parameters available:\n";
				std::cout << "\t" << argHelp0.data( ) << ", " << argHelp1.data( ) << ": Show parameters with their own arguments.\n";
				std::cout << "\t" << argWinSize.data( ) << " [width] [height]: Set the window size.\n";
				std::cout << "\t" << argFullscreen.data( ) << ": Run on the fullscreen mode.\n";

				return 0;
			}
			else if ( 0 == argWinSize.compare( cur ) )
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
				winWidth = subArg0;

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
				winHeight = subArg1;
			}
			else if ( 0 == argFullscreen.compare( cur ) )
			{
				winStyle |= sf::Style::Fullscreen;
			}
			else
			{
				std::cerr << "Error: There is no such parameter.\n";
				return -1;
			}
		}
	}
/*
=====
Initialization
=====
*/
	{
		::util::endian::BindConvertFunc( );
	}

/*
=====
Window
=====
*/
	sf::RenderWindow window( sf::VideoMode( winWidth, winHeight ), "Sirtet: the Classic", winStyle );
	window.setFramerateLimit( 60 );
	auto& console = *ServiceLocator::Console( );

	bool isOpen = true;
	while ( true == isOpen )
	{
		sf::Event event;
		while ( window.pollEvent( event ) )
		{
			if ( sf::Event::Closed == event.type )
			{
				isOpen = false;
			}
			else if ( sf::Event::KeyPressed == event.type )
			{
				if ( sf::Keyboard::Escape == event.key.code )
				{
					isOpen = false;
				}
				else if ( sf::Keyboard::Tab == event.key.code )
				{
					console.toggleVisible( );
				}
			}

			console.handleEvent( event );
		}

		///console.update( );

		window.clear( );
/*
=====
Window - Console
=====
*/
		if ( true == console.isVisible( ) )
		{
			window.draw( console );
		}

		window.display( );
	}

/*
=====
Resource Free
=====
*/
	window.close( );
}