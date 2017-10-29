#define TJH_DRAW_IMPLEMENTATION
#include "tjh_draw.h"

const int MAX_DATA = 100;

unsigned char data[MAX_DATA] = {};

void bubblePass( unsigned char* data, int endIndex )
{
	// Run over the data up to the given end index

	for( int i = 0; i < endIndex-1; i++ )
	{
		// Compare each element

		if( data[i] > data[i+1])
		{
			// If they are not in the correct order already, swap them
			// so the larger element comes after the smaller element.

			unsigned char temp = data[i];
			data[i] = data[i+1];
			data[i+1] = temp;
		}
	}
}

void bubbleSort( unsigned char* data, int length )
{
	// Run over the entire array repeatedly

	// After each pass, we know the largest element encountered is placed at
	// the end of the sorted area, and so doesn't need to be sorted again

	// This means the first pass covers the entire array,
	// the second pass covers the array minus the last value
	// (because we know it's the largest and already at the end),
	// the third pass can ignore the last two values,
	// and so on...

	for( int i = length; i > 0; i-- )
	{
		bubblePass(data, i);
	}
}

int main()
{
	// Make up some random data to sort

	for( int i = 0; i < MAX_DATA; i++ )
	{
		data[i] = rand() % 256;
	}

	// Do the bubble sort

	bubbleSort(data, MAX_DATA);

	// Everything below here just draws the data

	draw::init("bubble sort", MAX_DATA*4, 256);

	bool done = false;
	while( !done )
	{
		SDL_Event event;
		while( SDL_PollEvent( &event ) ) {
			if( event.type == SDL_QUIT ) done = true;
			else if( event.type == SDL_KEYDOWN
				&& event.key.keysym.scancode == SDL_SCANCODE_ESCAPE ) done = true;
		}

		draw::clear(0,0,0);
		draw::setColor(1);

		for( int i = 0; i < 100; i++ ) {
			draw::rect( i * 4, 256, 4, -data[i] );
		}

		draw::present();
	}

	draw::shutdown();
	return 0;
}