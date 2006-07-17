#ifndef __BT_TILEMANAGER_H__
#define __BT_TILEMANAGER_H__
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

#include "mrt/singleton.h"
#include "mrt/xml.h"
#include <map>

namespace sdlx {
	class Surface;
}

class AnimatedObject;

class IResourceManager : public mrt::XMLParser {
public:
	IResourceManager();
	~IResourceManager();
	DECLARE_SINGLETON(IResourceManager);
	
	void init(const std::string &fname);
	void clear();
	
	AnimatedObject *getAnimation(const std::string &id);

private:
	virtual void start(const std::string &name, Attrs &attr);
	virtual void end(const std::string &name);
	virtual void charData(const std::string &data);
	
	typedef std::map<const std::string, AnimatedObject *> AnimationMap;
	AnimationMap _animations;
	
	std::string _data;
	
	long _tw, _th;
};

SINGLETON(ResourceManager, IResourceManager);

#endif

