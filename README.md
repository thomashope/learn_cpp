# Learning C++

Notes on the C++ language and some choice libraries, to help me remember things.

## TODO

- SDL2
	- [ ] basic .bmp rendering
	- [ ] SDL2_Image (png, jpg, etc.)
	- [ ] basic OpenGL
	- [x] variable delta time
	- [ ] fixed delta time
	- [ ] basic sdl2 audio features (playing wav, volume maybe, anything else?)
	- [ ] controller input (PS3, PS4, XBox 360, XBox One, Wii?)
		- Rumble and audio out too?
		- What about the touch pad?
		- Motion sensors?
		- Button pressure?
- graphics
	- [x] floyd-steinbergh dithering
		- [x] ditch dependancy on SDL, take commmand line args and write result to file (stb_image_write or roll a PPM)
	- [ ] palletize an image. Take an input image and palette, output the image using only the palette colours
	- [x] stb_truetype font rendering with SDL
	- [x] stb_truetype font rendering with OpenGL
- 2d physics
	- [x] basic box2d example
	- [ ] box2d joints
	- [ ] 2d collision from scatch
- 3d physics
	- [x] basic bullet physics example
	- [ ] 3d collison from scatch
- scripting
	- [x] calling lua from c
	- [x] calling c from lua
	- [ ] try luaJIT?
	- [ ] passing more complex types?
- searching / sorting
	- [ ] lee
	- [ ] A*
	- [ ] bubble sort
	- [ ] heap sort
	- [ ] quick sort
	- [ ] binary search
	- [ ] radix sort
- inheritance
	- [ ] public, private, protected
	- [ ] virtual functions, override, final
	- [ ] multiple inheritence
- smart pointers
- curl
	- [x] downloading a web page / file
