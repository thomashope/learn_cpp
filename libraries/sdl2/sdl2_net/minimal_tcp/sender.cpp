#include <SDL2/sdl.h>
#include <SDL2/sdl_net.h>

// The following code example is placed in the public domain.
//
// Read more about SDLNet from the offical docs at http://sdl.beuc.net/sdl.wiki/SDL_net
//
// Decent simple tutorial on TCP with SDL_net http://headerphile.com/sdl2/sdl2-part-13-multiplayer-tcp/
//
// ~ tom 2017-08-27

// "127.0.0.1" represents localhost
#define SERVERIP "127.0.0.1"

// The port of the server that we shall send packets to
#define SERVERPORT 1616

int main()
{
	// First we have to initalize SDLNet itself
	if( SDLNet_Init() )
	{
		SDL_Log("Unable to initialize SDLNet: %s", SDLNet_GetError());
        return 1;
	}

	// Here we prepair the address object describing the destination
	// We will use the address object in later functions so SDLNet knows where to send the data
	IPaddress address;
	if( SDLNet_ResolveHost( &address, SERVERIP, SERVERPORT ) )
	{
		SDL_Log("Faild SDLNet_ResolveHost: %s", SDLNet_GetError());
		return 2;
	}

	// Open the desired port
	// This is where we actually attempt to make a connection with the recipient
	// Requires them having opened the same port for listening before we make this function call
	TCPsocket socket = SDLNet_TCP_Open( &address );
	if( socket == NULL )
	{
		SDL_Log("Failed to connect to server: %s", SDLNet_GetError());
		return 3;
	}
	else
	{
		SDL_Log("Connected to server %s:%i", SDLNet_ResolveIP(&address), SDLNet_Read16(&address.port));
	}

	const char message[] = "Hello TCP connection!";
	SDLNet_TCP_Send( socket, message, sizeof(message) );

	// Cleanup
	SDLNet_Quit();
	return 0;
}