#include <SDL2/sdl.h>
#include <SDL2/sdl_net.h>

// The following code example is placed in the public domain.
//
// Read more about SDLNet from the offical docs at http://sdl.beuc.net/sdl.wiki/SDL_net
//
// Decent simple tutorial on TCP with SDL_net http://headerphile.com/sdl2/sdl2-part-13-multiplayer-tcp/
//
// ~ tom 2017-08-27

// This is the port that we, the server, shall use to listen for and send packets on
#define SERVERPORT 1616

#define min(val1, val2) (val1 > val2 ? val2 : val1)

int main()
{
	// First we have to initalize SDLNet itself
	if( SDLNet_Init() )
	{
		SDL_Log("Unable to initialize SDLNet: %s", SDLNet_GetError());
        return 1;
	}

	// Set up the address and port that we will listen for connections on
	// The second parameter is an IP address. In our case we leave it as NULL
	// so SDLNet knows we want to listen for incoming connections
	IPaddress address;
	if( SDLNet_ResolveHost( &address, NULL, SERVERPORT ) )
	{
		SDL_Log("Faild SDLNet_ResolveHost: %s", SDLNet_GetError());
		return 2;
	}

	// Open the desired port and start listening for connections
	TCPsocket serverSocket = SDLNet_TCP_Open( &address );
	if( serverSocket == NULL )
	{
		SDL_Log("Failed to open port for listening: %s", SDLNet_GetError());
		return 3;
	}
	else
	{
		SDL_Log("Listening on port %i...", SERVERPORT);
	}

	// Now that we are listening, we need to wait for someone else to try and
	// open a connection with us. Insert an artificial delay here to give you
	// time to launch the other application :)
	SDL_Delay(5000);

	// Once someone tries to open a connection with us, we can accept it
	// This function returns a socket that represents the client that connected
	// If no one tried to connect (or there was an error) this will return NULL 
	TCPsocket clientSocket = SDLNet_TCP_Accept( serverSocket );
	if( clientSocket == NULL )
	{
		SDL_Log("Failed to connect to client: %s", SDLNet_GetError());
		return 4;
	}
	else
	{
		SDL_Log("Connected to client %s:%i", SDLNet_ResolveIP(&address), SDLNet_Read16(&address.port));
	}

	// A quit and dirty buffer for receiving data into
	const int MAXLEN = 256;
	char data[MAXLEN] = { 0 };

	// Now we can use the client socket and try receiving data from it into the buffer
	int bytesReceived = SDLNet_TCP_Recv( clientSocket, data, MAXLEN );

	// Ensure the data is null terminated before we print it out!
	data[bytesReceived] = 0;

	// Print out whatever it was we received
	SDL_Log("Received %i bytes: \"%s\"", bytesReceived, data );

	// Cleanup
	SDLNet_Quit();
	return 0;
}