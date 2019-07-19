#pragma hdrstop
#include <iostream>
#include <SFML/Graphics.hpp>
#include <Game/sequence/Sequence.h>
#include <Lib/Endian.h>
#include "ServiceLocator.h"

//TODO: config 파일 만들고 읽어오기
//TODO: 개발 완료 후 auto 쓰는 대신 타입 명시, 브랜치에 커밋

int main( int argc, char* argv[ ] )
{
	//궁금: 해시테이블의 reserve란?
	//variableTable.reserve( 10 );

	auto& variableTable = ServiceLocator::VariableTable( );
	{
		constexpr HashedKey key4 = util::hash::Digest( "debugLog" );
		variableTable.emplace( key4, true );
		//TODO: 이 3형제 필요 없을 듯
		constexpr HashedKey key0 = util::hash::Digest( "winWidth" );
		variableTable.emplace( key0, 800 );
		constexpr HashedKey key1 = util::hash::Digest( "winHeight" );
		variableTable.emplace( key1, 600 );
		constexpr HashedKey key2 = util::hash::Digest( "winStyle" );
		variableTable.emplace( key2, sf::Style::Close );
	}
/*
=====
Handling parameters on excution
=====
*/
	if ( 1 < argc )
	{
		const std::string argHelp0( "--help" );
		const std::string argHelp1( "--h" );
		const std::string argWinSize( "--WS" );
		const std::string argFullscreen( "--FS" );

		for ( int i = 1; i < argc; ++i )
		{
			const char* cur = argv[ i ];
			// When "--help" or "--h",
			if ( 0 == argHelp0.compare( cur ) || 0 == argHelp1.compare( cur ) )
			{
				std::cout << "Here are case-sensitive parameters available:\n";
				std::cout << "\t" << argHelp0.data( ) << ", " << argHelp1.data( ) << ": Show parameters with their own arguments.\n";
				std::cout << "\t" << argWinSize.data( ) << " [width] [height]: Set the window size.\n";
				std::cout << "\t" << argFullscreen.data( ) << ": Run on the fullscreen mode.\n";

				return 0;
			}
			// When "--WS",
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
				constexpr HashedKey key0 = util::hash::Digest( "winWidth" );
				variableTable.find( key0 )->second = subArg0;

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
				constexpr HashedKey key1 = util::hash::Digest( "winHeight" );
				variableTable.find( key1 )->second = subArg1;
			}
			// When "--FS",
			else if ( 0 == argFullscreen.compare( cur ) )
			{
				constexpr HashedKey key = util::hash::Digest( "winStyle" );
				variableTable.find( key )->second |= sf::Style::Fullscreen;
			}
			// When an undefined parameter has been passed,
			else
			{
				std::cerr << "Error: There is no such parameter.\n";
				return -1;
			}
		}
	}
/*
=====
Lazy Initialization
=====
*/
	{
		::util::endian::BindConvertFunc( );
		::global::Console = &ServiceLocator::Console;
		::global::VariableTable = &ServiceLocator::VariableTable;
		constexpr HashedKey key0 = util::hash::Digest( "foreFPS" );
		variableTable.emplace( key0, 60u );
		constexpr HashedKey key1 = util::hash::Digest( "backFPS" );
		variableTable.emplace( key1, 30u );
	}

/*
=====
Window
=====
*/
	constexpr HashedKey key0 = util::hash::Digest( "winWidth" );
	constexpr HashedKey key1 = util::hash::Digest( "winHeight" );
	constexpr HashedKey key2 = util::hash::Digest( "winStyle" );
	sf::RenderWindow window( sf::VideoMode( variableTable.find( key0 )->second,
											variableTable.find( key1 )->second ),
							 "Sirtet: the Classic",
							 variableTable.find( key2 )->second );
	constexpr HashedKey key3 = util::hash::Digest( "foreFPS" );
	window.setFramerateLimit( variableTable.find( key3 )->second );
	// !IMPORTANT: Now console has been initialized.
	auto& console = *ServiceLocator::ConsoleWithInit( window.getSize( ) );
	::sequence::Sequence game( window );
	
	bool isOpen = true;
	do
	{
		sf::Event event;
		while ( true == window.pollEvent( event ) )
		{
			if ( sf::Event::Closed == event.type )
			{
				isOpen = false;
			}
			else if ( sf::Event::KeyPressed == event.type )
			{
				if ( sf::Keyboard::Escape == event.key.code )
				{
					if ( false == console.isVisible( ) )
					{
						isOpen = false;
					}
					// else ... is dealt with in 'console.handleEvent( event ).'
				}
			}
			else if ( sf::Event::LostFocus == event.type )
			{
				constexpr HashedKey key4 = util::hash::Digest( "backFPS" );
				window.setFramerateLimit( variableTable.find( key4 )->second );
			}
			else if ( sf::Event::GainedFocus == event.type )
			{
				window.setFramerateLimit( variableTable.find( key3 )->second );
			}
					
			// Console
			console.handleEvent( event );
		}

		if ( false == isOpen )
		{
			break;
		}

		game.update( );

		window.clear( );
		game.draw( );
		if ( true == console.isVisible( ) )
		{
			window.draw( console );
		}
		window.display( );
	}
	while ( true );

/*
=====
Resource Free
=====
*/
	window.close( );
}