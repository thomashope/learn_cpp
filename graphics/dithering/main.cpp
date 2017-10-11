// #define TJH_DRAW_IMPLEMENTATION
// #include "tjh_draw.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

typedef unsigned char u8;

// Colour palette to use for dithering
u8 palette[] = 
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

struct Colour
{
	u8 r, g, b;

	Colour operator * (float f )
	{
		return { u8(r * f), u8(g * f), u8(b * f) };
	}
	Colour operator - (const Colour& rhs)
	{
		return { (u8)clamp(r - rhs.r, 0, 255),
			(u8)clamp(g - rhs.g, 0, 255),
			(u8)clamp(b - rhs.b, 0, 255) };
	}
	Colour operator + (const Colour& rhs)
	{
		return { (u8)clamp(r + rhs.r, 0, 255),
			(u8)clamp(g + rhs.g, 0, 255),
			(u8)clamp(b + rhs.b, 0, 255) };

	}
};

Colour getColour( u8* image, int width, int x, int y )
{
	return { image[ y * width * 3 + x * 3 ],
		image[ y * width * 3 + x * 3 + 1 ],
		image[ y * width * 3 + x * 3 + 2 ] };
}

void setColour( u8* image, int width, int x, int y, Colour c )
{
	image[ y * width * 3 + x * 3 ] = c.r;
	image[ y * width * 3 + x * 3 + 1 ] = c.g;
	image[ y * width * 3 + x * 3 + 2 ] = c.b;
}

void addColour( u8* image, int width, int x, int y, Colour c )
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

int main( int argc, char* argv[] )
{
	// Use snow.jpg by default, or get from the command line
	const char* filename = "snow.jpg";
	if( argc > 1 )
		filename = argv[1];

	int width, height, c;
	u8* original_image = stbi_load( filename, &width, &height, &c, 3 );
	u8* dithered_image = new u8[width * height * 3];

	// This ditheing implementation snakes form left to right to hopefully spread
	// the error around a bit more evenly
	//
	// dir indicates the current direction, positive is to the right
	int dir = 1;
	
	for( int y = 0; y < height; y++ )
	{
		// A little ugly, x will either increase or decrease
		// depending on what direction we are currently going in
		for( int x = (dir > 0 ? 0 : width - 1);
			x != (dir > 0 ? width : -1);
			x += dir )
		{
			Colour original = getColour( original_image, width, x, y );

			// Get the closset colour form the palette (defined at the top of the file)
			Colour closest  = getClosest( original );


			setColour( dithered_image, width, x, y, closest );

			// Take the error and distribute it over nearby pixels
			// This pattern was just copied from wikipedia, there may be other patterns that
			// produce different effects.
			// https://en.wikipedia.org/wiki/Floyd–Steinberg_dithering

			Colour error = original - closest;

			// Also, before we write to the image we check the pixel is actually in range
			// and we aren't writing off the edge or something
			if( inRange(x + dir, 0, width - 1) )
			{
				addColour( original_image, width, x + dir, y, error * (7.0f/16.0f) );
			}

			if( inRange(x + dir, 0, width - 1)
				&& y < height - 1 )
			{
				addColour( original_image, width, x - dir, y + dir, error * (3.0f/16.0f) );
			}

			if( inRange(y, 0, height - 1) )
			{
				addColour( original_image, width, x      , y + dir, error * (5.0f/16.0f) );
			}

			if( inRange(x + dir, 0, width - 1)
				&& y < height - 1)
			{
				addColour( original_image, width, x + dir, y + dir, error * (1.0f/16.0f) );
			}
		}

		// Switch direction
		dir *= -1;
	}

	const char* prefix = "dithered_";
	int size = (argc > 1 ? strlen(argv[1]) : 0) + strlen(prefix);
	char outName[size];
	sprintf( outName, "%s%s", prefix, filename );

	stbi_write_png( outName, width, height, 3, dithered_image, width * 3 );

	delete[] dithered_image;
	stbi_image_free(original_image);

	return 0;
}