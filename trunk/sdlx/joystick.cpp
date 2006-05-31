#include "SDL/SDL.h"

#include "sdlx/joystick.h"
#include "sdlx/sdl_ex.h"

using namespace sdlx;

const int Joystick::getCount() {
	const int c = SDL_NumJoysticks();
	if (c < 0)
		throw_sdl(("SDL_NumJoysticks"));
	return c;
}

const std::string Joystick::getName(const int idx) {
	const char * name = SDL_JoystickName(idx);
	if (name == NULL)
		throw_sdl(("SDL_JoystickName(%d)", idx));
	return name;
}

void Joystick::sendEvents(const bool enable) {
	SDL_JoystickEventState(enable ? SDL_ENABLE: SDL_IGNORE);
}


Joystick::Joystick() : _joy(NULL) {}

void Joystick::open(const int idx) {
	close();
	_joy = SDL_JoystickOpen(idx);
	if (_joy == NULL)
		throw_sdl(("SDL_JoystickOpen(%d)", idx));
}

void Joystick::close() {
	if (_joy == NULL) 
		return;
	
	SDL_JoystickClose(_joy);
	_joy = NULL;
}

Joystick::~Joystick() {
	close();
}
