#include "pch.h"
#include "Console.h"

class ConsoleLocal : public IConsole
{
public:
	ConsoleLocal( )
	{
		//TODO:
		std::cout << "Is ConsoleLocal instance really dynamically bind?\n";
	}
	~ConsoleLocal( ) = default;
};

std::unique_ptr< IConsole > Console = std::make_unique< ConsoleLocal >( );

int main( int argc, char* argv[ ] )
{
	const std::string argHelp0( "-help" );
	const std::string argHelp1( "-h" );
	const std::string argWinSize( "-WS" );
	const std::string argFullscreen( "-FS" );
	int width = 800;
	int height = 600;
	bool isFullScreen = false;

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


}