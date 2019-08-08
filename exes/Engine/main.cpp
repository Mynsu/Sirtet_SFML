#pragma hdrstop
#include <Lib/precompiled.h>
#include <iostream>
#include <windows.h>
#include "Game/Game.h"
#include "ServiceLocator.h"

//TODO: 개발 완료 후 auto 쓰는 대신 타입 명시, 브랜치에 커밋

int main( int argc, char* argv[ ] )
{
	uint16_t winWidth = 800u;
	uint16_t winHeight = 600u;
	uint8_t winStyle = sf::Style::Close;
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
			// When "--FS",
			else if ( 0 == argFullscreen.compare( cur ) )
			{
				winStyle |= sf::Style::Fullscreen;
			}
			// Exception: When an undefined parameter has been passed,
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
	::util::endian::BindConvertFunc( );

	const uint16_t FOREGROUND_FPS = 60u;
	auto& variableTable = ServiceLocator::Vault( );
	constexpr HashedKey HK_FORE_FPS = util::hash::Digest( "foreFPS" ); //TODO: 한 프레임 안에 계산할 필요가 없는 게 뭐가 있을까?
	variableTable.emplace( HK_FORE_FPS, FOREGROUND_FPS );
	constexpr HashedKey HK_BACK_FPS = util::hash::Digest( "backFPS" );
	variableTable.emplace( HK_BACK_FPS, 30u );

	HMODULE hGameDLL = LoadLibraryA( "game.dll" );
	if ( nullptr == hGameDLL )
	{
		std::cerr << "Fatal failure: Failed to load 'game.dll.'" << std::endl;
		return -1;
	}
	GetGameAPI_t getGameAPI = reinterpret_cast< GetGameAPI_t >( GetProcAddress( hGameDLL, "GetGameAPI" ) );
	if ( nullptr == getGameAPI )
	{
		std::cerr << "Fatal failure: Failed to get the address of the function 'GetGameAPI.'" << std::endl;
		FreeLibrary( hGameDLL );
		return -1;
	}
	EngineComponents engineComponents;
	engineComponents.console = &ServiceLocator::Console;
	engineComponents.vault = &ServiceLocator::Vault;
	sf::RenderWindow window( sf::VideoMode( winWidth, winHeight ),
							 "Sirtet: the Classic",
							 winStyle );
	window.setFramerateLimit( FOREGROUND_FPS );
	engineComponents.window = &window;
	const GameComponents gameComponents = getGameAPI( engineComponents );
	// Passed by value, or copied, thus initialized for the security.
	engineComponents = { nullptr };
/*
=====
Main Loop
=====
*/
	IConsole& console = *ServiceLocator::Console( );
	bool isOpen = true;
	while ( true == isOpen )
	{
		sf::Event event;
		while ( true == window.pollEvent( event ) )
		{
			if ( sf::Event::Closed == event.type )
			{
				isOpen = false;
				break;
			}
			else if ( sf::Event::KeyPressed == event.type )
			{
				if ( sf::Keyboard::Escape == event.key.code )
				{
					if ( false == console.isVisible( ) )
					{
						isOpen = false;
						break;
					}
					// else ... is dealt with in 'console.handleEvent( event ).'
				}
			}
			else if ( sf::Event::LostFocus == event.type )
			{
				window.setFramerateLimit( variableTable.find( HK_BACK_FPS )->second );
			}
			else if ( sf::Event::GainedFocus == event.type )
			{
				window.setFramerateLimit( variableTable.find( HK_FORE_FPS )->second );
			}
					
			// Console
			console.handleEvent( event );
		}

		gameComponents.game->update( );

		window.clear( );
		gameComponents.game->draw( );
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
	// !IMPORTANT: Let .dll free only after calling 'ServiceLocator::Release( )',
	//			   otherwise this would try to access a function in .dll through a pointer,
	//			   which is violation and makes an exception happen.
	ServiceLocator::Release( );
	window.close( );
	FreeLibrary( hGameDLL );
}