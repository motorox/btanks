#ifndef __BT_JOYPLAYER_H__
#define __BT_JOYPLAYER_H__
/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "player.h"
#include "sdlx/joystick.h"

class JoyPlayer :public Player {
public:
	JoyPlayer(const std::string &animation, const int idx, const int fire);
	virtual void tick(const float dt);
	
	virtual ~JoyPlayer();
private:
	sdlx::Joystick _joy;
	int _fire;
};

#endif

