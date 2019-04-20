#include <iostream>
#include <string>

using hash = uint32_t;
constexpr size_t MAX_ARG_LEN = 15;

const hash generateHash( const char* arg, size_t len )
{
	hash retHash = 0u;
	for ( size_t i = 0; i != len; ++i )
	{
		retHash += 65599 * retHash + arg[ i ];
	}

	return retHash ^ ( retHash >> 16 );
}

int main()
{
	char arg[ MAX_ARG_LEN ] = { 0 };
	while ( true )
	{
		std::cin >> arg;
		std::cout << strlen( arg ) << " | " << generateHash( arg, strlen( arg ) ) << " generated.\n";
		arg[ 0 ] = '\0';
	}
}