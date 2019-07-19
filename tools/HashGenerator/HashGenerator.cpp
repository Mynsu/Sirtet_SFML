#include <iostream>
#include <string>
#include <windows.h> // HGLOBAL, GlobalAlloc( ... ), GlobalLock( ... ), GlobalUnlock( ... )

using HashValue = uint32_t;
constexpr size_t MAX_ARG_LEN = 15;

const HashValue digest( const char* arg, size_t len )
{
	HashValue retVal = 0u;
	for ( size_t i = 0; i != len; ++i )
	{
		retVal += 65599u * retVal + arg[ i ];
	}

	return retVal ^ ( retVal >> 16 );
}

int main()
{
	char arg[ MAX_ARG_LEN ] = { 0 };
	while ( true )
	{
		// NOTE: Wait until key 'ENTER' is pressed down. This isn't busy wait.
		std::cin >> arg;
		const HashValue hashVal = digest( arg, strlen( arg ) );
		std::cout << strlen( arg ) << " | " << hashVal << " generated and copied to clipboard.\n";
		// Copy to clipboard
		const std::string str( std::to_string( hashVal ) );
		const size_t strLen = str.length( ) + 1;
		HGLOBAL handle = GlobalAlloc( GMEM_MOVEABLE, strLen );
		memcpy( GlobalLock( handle ), str.data(	), strLen );
		GlobalUnlock( handle );
		OpenClipboard( NULL );
		EmptyClipboard( );
		SetClipboardData( CF_TEXT, handle );
		CloseClipboard( );
		// Initialize the buffer for the next input.
		arg[ 0 ] = '\0';
	}
}