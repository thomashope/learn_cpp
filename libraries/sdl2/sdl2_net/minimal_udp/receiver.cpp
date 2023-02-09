#include <SDL2/sdl.h>
#include <SDL2/sdl_net.h>

// The following code example is placed in the public domain.
//
// Read more about SDLNet from the offical docs at http://sdl.beuc.net/sdl.wiki/SDL_net
//
// Decent simple tutorial on UDP with SDL_net http://headerphile.com/sdl2/sdl2-part-12-multiplayer/
//
// ~ tom 2017-08-27

// "127.0.0.1" represents localhost
#define IP "127.0.0.1"

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

	// Open the desired port for communicating of UDP
	UDPsocket socket = SDLNet_UDP_Open( SERVERPORT );
	if( socket == NULL )
	{
		SDL_Log("Failed SDLNet_UDP_Open: %s", SDLNet_GetError());
		return 2;
	}
	else
	{
		SDL_Log("Listening on port %i...", SERVERPORT);
	}

	// Ask SDLNet to allocate a packet structure which we will later receive data into
	const int PACKET_SIZE = 256;
	UDPpacket* packet = SDLNet_AllocPacket(PACKET_SIZE);
	if( packet == NULL )
	{
		SDL_Log("Failed SDLNet_AllockPacket: %s", SDLNet_GetError());
		return 3;
	}

	bool received = false;

	while(!received)
	{
		// Check the socket for incoming packets
		int recv_result = SDLNet_UDP_Recv( socket, packet );
		if( recv_result > 0 )
		{
			// The address field of the packet contains the source address
			SDL_Log("Received data from %s:%i", SDLNet_ResolveIP(&packet->address), SDLNet_Read16(&packet->address.port));
			
			// Ensure the data is null terminated so someone doesn't hack us!
			packet->data[ packet->maxlen-1 ] = 0;

			// Print out the contents of the packet
			SDL_Log( "Data: \"%s\"", packet->data );

			// Set received to true so we can exit the loop
			received = true;
		}
		else if( recv_result < 0 )
		{
			SDL_Log( "Failed SDLNet_UDP_Recv" );
			return 5;
		}

		// If we didn't receive anything, wait so we don't max out the CPU
		SDL_Delay(200);
	}

	// Cleanup
	SDLNet_FreePacket(packet); // Don't forget to free the packet!
	SDLNet_Quit();
	return 0;
}