#define TJH_DRAW_IMPLEMENTATION
#include "tjh_draw.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <algorithm>

int clamp( int v, int lo, int hi )
{
	if( v < lo ) return lo;
	if( v > hi ) return hi;
	return v;
}

bool inRange( int v, int lo, int hi )
{
	return v >= lo && v <= hi;
}

// Colour palette to use for dithering
//*/
Uint8 palette[] = 
{
	255, 255, 255,
	255,   0,   0,
	  0, 255,   0,
	  0,   0, 255,
	  0, 255, 255,
	255,   0, 255,
	255, 255,   0,
	  0,   0,   0
};
//*/

/*/
Uint8 palette[] = 
{
	250, 250, 250,
	240, 240, 240,
	230, 230, 230,
	220, 220, 220,
	210, 210, 210,
	200, 200, 200,
	190, 190, 190,
	180, 180, 180,
	170, 170, 170,
	150, 150, 150,
	130, 130, 130,
	100, 100, 100,
	 70,  70,  70,
	 50,  50,  50,
	 30,  30,  30,
	  0,   0,   0
};
//*/

struct Colour
{
	Uint8 r, g, b;

	Colour operator * (float f )
	{
		return { Uint8(r * f), Uint8(g * f), Uint8(b * f) };
	}
	Colour operator - (const Colour& rhs)
	{
		return { (Uint8)clamp(r - rhs.r, 0, 255),
			(Uint8)clamp(g - rhs.g, 0, 255),
			(Uint8)clamp(b - rhs.b, 0, 255) };
	}
	Colour operator + (const Colour& rhs)
	{
		return { (Uint8)clamp(r + rhs.r, 0, 255),
			(Uint8)clamp(g + rhs.g, 0, 255),
			(Uint8)clamp(b + rhs.b, 0, 255) };

	}
};

Colour getColour( Uint8* image, int width, int x, int y )
{
	return { image[ y * width * 3 + x * 3 ],
		image[ y * width * 3 + x * 3 + 1 ],
		image[ y * width * 3 + x * 3 + 2 ] };
}

void setColour( Uint8* image, int width, int x, int y, Colour c )
{
	image[ y * width * 3 + x * 3 ] = c.r;
	image[ y * width * 3 + x * 3 + 1 ] = c.g;
	image[ y * width * 3 + x * 3 + 2 ] = c.b;
}

void addColour( Uint8* image, int width, int x, int y, Colour c )
{
	Colour dest = getColour( image, width, x, y );

	image[ y * width * 3 + x * 3 ] += c.r;
	image[ y * width * 3 + x * 3 + 1 ] += c.g;
	image[ y * width * 3 + x * 3 + 2 ] += c.b;
}

Colour getClosest( Colour target )
{
	Colour result;
	float closest = -1;
	for( int i = 0; i < sizeof(palette)/sizeof(*palette); i+=3)
	{
		int dist_r = (target.r - palette[i]);
		int dist_g = (target.g - palette[i+1]);
		int dist_b = (target.b - palette[i+2]);

		float dist_squared = dist_r * dist_r + dist_g * dist_g + dist_b * dist_b;
		if( closest < 0 or dist_squared < closest)
		{
			result.r = palette[i];
			result.g = palette[i+1];
			result.b = palette[i+2];
			closest = dist_squared;
		}
	}
	return result;
}

int main()
{
	int width, height, c;
	unsigned char * image_buffer = stbi_load("snow.jpg", &width, &height, &c, 3);
	unsigned char * dithered_buffer = new unsigned char[width * height * 3];

	// We will snake form left to right
	// dir indicates the current direction, positive is right
	int dir = 1;
	
	for( int y = 0; y < height; y++ )
	{
		for( int x = (dir > 0 ? 0 : width - 1);
			x != (dir > 0 ? width : -1);
			x += dir )
		{
			Colour original = getColour( image_buffer, width, x, y );
			Colour closest  = getClosest( original );

			setColour( dithered_buffer, width, x, y, closest );

			// Take the error and distribute it over nearby pixels
			// Specifically those to the right and below that are yet to be processed

			Colour error = original - closest;

			if( inRange(x + dir, 0, width - 1) )
			{
				addColour( image_buffer, width, x + dir, y, error * (7.0f/16.0f) );
			}

			if( inRange(x + dir, 0, width - 1)
				&& y < height - 1 )
			{
				addColour( image_buffer, width, x - dir, y + dir, error * (3.0f/16.0f) );
			}

			if( inRange(y, 0, height - 1) )
			{
				addColour( image_buffer, width, x      , y + dir, error * (5.0f/16.0f) );
			}

			if( inRange(x + dir, 0, width - 1)
				&& y < height - 1)
			{
				addColour( image_buffer, width, x + dir, y + dir, error * (1.0f/16.0f) );
			}
		}

		// Switch direction
		dir *= -1;
	}

	draw::init( __FILE__, width, height );

	SDL_Event event;
	bool done = false;
	while( !done )
	{
		while( SDL_PollEvent(&event) )
		{
			if( event.type == SDL_QUIT ) done = true;
		}
		draw::clear(0, 0.5, 0);

		for( int y = 0; y < height; y++ )
		{
			for( int x = 0; x < width; x++ )
			{
				float r = dithered_buffer[ y * width * 3 + x * 3 ] / 255.0f;
				float g = dithered_buffer[ y * width * 3 + x * 3 + 1 ] / 255.0f;
				float b = dithered_buffer[ y * width * 3 + x * 3 + 2 ] / 255.0f;

				draw::setColor( r, g, b );
				draw::point( x, y );
			}
		}

		draw::present();
	}

	delete[] dithered_buffer;
	stbi_image_free(image_buffer);
	draw::shutdown();
	return 0;
}