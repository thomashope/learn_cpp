#define TJH_DRAW_IMPLEMENTATION
#include "../tjh_draw.h"

#include "../tjh_math.h"

const int WIDTH = 1280;
const int HEIGHT = 720;

struct circle {
	vec2 pos;
	float radius;

	void draw() { draw::circle(pos.x, pos.y, radius); }
};

struct line {
	float x1, y1, x2, y2;

	void draw() { draw::line(x1, y1, x2, y2); }
};

float dist( float x1, float y1, float x2, float y2 )
{
	return (vec2(x1, y1) - vec2(x2, y2)).length();
}

bool isTouching( const circle& c, const line& l )
{	
	vec2 line( l.x2 - l.x1, l.y2 - l.y1 );
	vec2 lineNormalized = line.normalized();

	vec2 lineToCircle( c.pos.x - l.x1, c.pos.y - l.y1 );

	float closestDist = lineToCircle.dot( lineNormalized );
	vec2 closestPoint = vec2(l.x1, l.y1) + lineNormalized * closestDist;

	draw::circle( closestPoint.x, closestPoint.y, 10 );

	if( closestDist < 0 )
	{
		return c.pos.distance( {l.x1, l.y1} ) < c.radius;
	}
	else if( closestDist > line.length() )
	{
		return c.pos.distance( {l.x2, l.y2} ) < c.radius;
	}

	return closestPoint.distance( c.pos ) < c.radius;
}

int main()
{
	draw::init("collision", WIDTH, HEIGHT );

	char buf[256];	
	int textHeight = 12;

	int mouseX, mouseY;

	circle c1{{0, 0}, 30};
	line l1{200, 200, 500, 400};

	bool done = false;
	while( !done )
	{
		SDL_GetMouseState( &mouseX, &mouseY );

		SDL_Event event;
		while( SDL_PollEvent( &event ) ) {
			if( event.type == SDL_QUIT ) done = true;
			else if( event.type == SDL_KEYDOWN
				&& event.key.keysym.scancode == SDL_SCANCODE_ESCAPE ) done = true;
		}

		draw::clear( 0.1, 0.1, 0.1 );

		c1.pos.x = mouseX;
		c1.pos.y = mouseY;

		if( isTouching( c1, l1 ) )
		{
			draw::setColor( 0.9, 0.2, 0.2 );
		}
		else
		{
			draw::setColor( 0.9 );
		}

		c1.draw();

		draw::setColor( 1 );

		sprintf( buf, "x: %d, y: %d",  int(c1.pos.x), int(c1.pos.y) );
		draw::text( buf, c1.pos.x + c1.radius + 10, c1.pos.y, textHeight );

		vec2 lineToCircle = c1.pos - vec2(l1.x1, l1.y1);
		draw::setColor( 0.9, 0.2, 0.9 );
		draw::line( l1.x1, l1.y1, l1.x1 + lineToCircle.x, l1.y1 + lineToCircle.y );

		sprintf( buf, "angle: %.2f", lineToCircle.angle( vec2(l1.x2-l1.x1, l1.y2-l1.y1) ) * RAD_TO_DEG );
		draw::text( buf, c1.pos.x + c1.radius + 10, c1.pos.y + textHeight, textHeight );

		draw::setColor( 0.9 );
		l1.draw();

		draw::present();
	}

	draw::shutdown();
	return 0;
}