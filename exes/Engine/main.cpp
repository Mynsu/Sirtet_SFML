#include "pch.h"
#include "../VaultKeyList.h"
#include "ServiceLocator.h"

const uint32_t DEFAULT_FOREGROUND_FPS = 60;
const uint32_t DEFAULT_BACKGROUND_FPS = 60;

int main( int argc, char* argv[] )
{
	uint32_t winWidth = 800;
	uint32_t winHeight = 600;
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
			const char* const cur = argv[ i ];
			// When "--help" or "--h",
			if ( 0==argHelp0.compare(cur) || 0==argHelp1.compare(cur) )
			{
				std::cout << "Here are case-sensitive parameters available:\n";
				std::cout << "\t" << argHelp0.data( ) << ", " << argHelp1.data( ) << ": Show parameters with their own arguments.\n";
				std::cout << "\t" << argWinSize.data( ) << " [width] [height]: Set the window size.\n";
				std::cout << "\t" << argFullscreen.data( ) << ": Run on the fullscreen mode.\n";

				return 0;
			}
			// When "--WS",
			else if ( 0 == argWinSize.compare(cur) )
			{
				const int subArg0 = std::atoi( argv[++i] );
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

				const int subArg1 = std::atoi( argv[++i] );
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
			else if ( 0 == argFullscreen.compare(cur) )
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
	auto& vault = gService.vault( );
	vault.emplace( HK_FORE_FPS, DEFAULT_FOREGROUND_FPS );
	vault.emplace( HK_BACK_FPS, DEFAULT_BACKGROUND_FPS );
	Console& console = gService._console( );
	console.initialize( );

	HMODULE hGameDLL = LoadLibraryA( "game.dll" );
	// File Not Found Exception
	if ( nullptr == hGameDLL )
	{
		std::cerr << "FATAL: Failed to load 'game.dll.'" << std::endl;
		return -1;
	}
	const std::string procedureName( "GetGameAPI" );
	GetGameAPI_t GetGameAPI = (GetGameAPI_t)GetProcAddress( hGameDLL, procedureName.data() );
	// Exception: When function 'GetGameAPI(...)' isn't declared with 'extern "C"' keyword or not registered in .def file.
	if ( nullptr == GetGameAPI )
	{
		std::cerr << "FATAL: Failed to get the address where the procedure in dll. is" << std::endl;
		FreeLibrary( hGameDLL );
		return -1;
	}
	EngineComponents engineComponents;
	engineComponents.service = &gService;
	sf::RenderWindow window( sf::VideoMode( winWidth, winHeight ),
							 "Sirtet: the Classic",
							 winStyle );
	window.setFramerateLimit( DEFAULT_FOREGROUND_FPS );
	engineComponents.window = &window;
	const GameComponents gameComponents = GetGameAPI( engineComponents );
	// Passed by value, or copied, thus initialized for security.
	engineComponents = { nullptr };
/*
=====
Main Loop
=====
*/
	vault.emplace( HK_IS_RUNNING, 1 );
	vault.emplace( HK_HAS_GAINED_FOCUS, 1 );
	while ( 0 != vault[HK_IS_RUNNING] )
	{
		std::vector<sf::Event> subEventQueue;
		sf::Event event;
		while ( true == window.pollEvent(event) )
		{
			if ( sf::Event::Closed == event.type )
			{
				vault[ HK_IS_RUNNING ] = 0;
				break;
			}
			else if ( sf::Event::LostFocus == event.type )
			{
				window.setFramerateLimit( vault.find(HK_BACK_FPS)->second );
				gService.vault( )[ HK_HAS_GAINED_FOCUS ] = 0;
			}
			else if ( sf::Event::GainedFocus == event.type )
			{
				window.setFramerateLimit( vault.find(HK_FORE_FPS)->second );
				gService.vault( )[ HK_HAS_GAINED_FOCUS ] = 1;
			}
			else if ( 0 == vault[HK_HAS_GAINED_FOCUS] )
			{
				break;
			}
			else
			{
				subEventQueue.emplace_back( event );
			}
					
			// Console
			console.handleEvent( subEventQueue );
		}

		gameComponents.game->update( subEventQueue );

		// NOTE: Skipped.
		///subEventQueue.clear();
		
		window.clear( );
		gameComponents.game->draw( );
		if ( true == console.isVisible() )
		{
			console.draw( window );
		}
		window.display( );
	}

/*
=====
Resource Free
=====
*/
	// !IMPORTANT: Let .dll free only after having destructed Command System,
	//			   otherwise Command System would try to access and delete function pointers in and of .dll,
	//			   which is violation and makes an exception happen.
	gService.release( );
	window.close( );
	FreeLibrary( hGameDLL );
}