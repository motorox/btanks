#ifndef BTANKS_MENU_MEDALS_H__
#define BTANKS_MENU_MEDALS_H__

#include "container.h"

class Medals : public Container {
public: 
	Medals(int w, int h); 
	bool onKey(const SDL_keysym sym);
};

#endif

