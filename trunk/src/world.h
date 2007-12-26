#ifndef __BT_WORLD_H__
#define __BT_WORLD_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
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

#include "export_btanks.h"
#include "mrt/singleton.h"
#include "mrt/serializable.h"

#include <set>
#include <map>

#include "math/v2.h"
#include "math/matrix.h"
#include "object_common.h"

#include "object_grid.h"
#include "utils.h"
#include <sigc++/sigc++.h>

namespace sdlx {
class Surface;
class Rect;
}

namespace ai {
class Traits;
}

class Object;

class BTANKSAPI IWorld : public mrt::Serializable, public sigc::trackable {
public:
	DECLARE_SINGLETON(IWorld);

	void clear();
	~IWorld();
	IWorld();
	
	void setTimeSlice(const float ts);
	
	void addObject(Object *, const v2<float> &pos, const int id = -1);
	const bool exists(const int id) const;
	const Object *getObjectByID(const int id) const;
	Object *getObjectByID(const int id);
	
	void render(sdlx::Surface &surface, const sdlx::Rect &src, const sdlx::Rect &viewport, const int z1 = -10000, const int z2 = 10001, const Object * player = NULL);
	void tick(const float dt);
	
	Object * spawn(const Object *src, const std::string &classname, const std::string &animation, const v2<float> &dpos, const v2<float> &vel, const int z = 0);

//the nearest objects
	const Object* getNearestObject(const Object *obj, const std::set<std::string> &classnames, const float range, const bool check_shooting_range) const;
	const bool getNearest(const Object *obj, const std::set<std::string> &classnames, const float range, v2<float> &position, v2<float> &velocity, const bool check_shooting_range) const;
//end of the nearest

	const bool old_findPath(const Object *obj, const v2<float>& position, Way & way, const Object * dst = NULL) const;
	void getImpassabilityMatrix(Matrix<int> &matrix, const Object *src, const Object *dst) const;

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
	void generateUpdate(mrt::Serializator &s, const bool clean_sync_flag);
	void applyUpdate(const mrt::Serializator &s, const float dt);

	void serializeObject(mrt::Serializator &, const Object *, const bool force) const;
	Object* deserializeObject(const mrt::Serializator &);
	void serializeObjectPV(mrt::Serializator &, const Object *) const;
	void deserializeObjectPV(const mrt::Serializator &, Object *);

	static void interpolateObject(Object *object);
	static void interpolateObjects(ObjectMap &objects);

	void tick(Object &o, const float dt, const bool do_calculate = true);
	void tick(ObjectMap &objects, const float dt, const bool do_calculate = true);
	void purge(ObjectMap &objects, const float dt);
	void purge(const float dt);
	
	void setSafeMode(const bool safe_mode = true);
	inline const bool safeMode() const { return _safe_mode; }

	const float getImpassability(Object *obj, const v2<int> &position, const Object **collided_with = NULL, const bool probe = false, const bool skip_moving = false) const;
	
	const int getChildren(const int id, const std::string &classname) const;
	const bool itemExists(const std::set<std::string> &classes) const;
	void setMode(const std::string &mode, const bool value);
	
	const bool attachVehicle(Object *object, Object *vehicle);
	const bool detachVehicle(Object *object);

	void enumerateObjects(std::set<const Object *> &o_set, const Object *src, const float range, const std::set<std::string> *classfilter);

protected: 
	friend class Editor;
	friend class Command;
	
	const Object *getObjectByXY(const int x, const int y) const;
	void move(const Object *object, const int x, const int y);
	
private:
	void _tick(Object &o, const float dt, const bool do_calculate = true);
	void _tick(ObjectMap &objects, const float dt, const bool do_calculate = true);

	void initMap();
	
	void updateObject(const Object *o);
	void deleteObject(const Object *o);
	
	void replaceID(const int old_id, const int new_id);
	
	typedef std::map<const std::pair<int, int>, bool> CollisionMap;
	mutable CollisionMap _collision_map;

	typedef std::map<const std::pair<int, int>, ternary<int, int, bool> > StaticCollisionMap;
	mutable StaticCollisionMap _static_collision_map;
	
	//void getImpassability2(float &old_pos_im, float &new_pos_im, Object *obj, const v2<int> &new_position, const Object **old_pos_collided_with = NULL) const;
	const bool collides(Object *obj, const v2<int> &position, Object *other, const bool probe = false) const;

	
	void cropObjects(const std::set<int> &ids);
	
	void setSpeed(const float speed);

	ObjectMap _objects;
	Grid _grid; //hopefully it'll be our saviour.
	int _last_id;
	bool _safe_mode, _atatat;
	float _max_dt;
	
	const sdlx::Surface *_hp_bar;

	IWorld(const IWorld &);
	const IWorld& operator=(const IWorld &);
};

SINGLETON(BTANKSAPI, World, IWorld);

#endif
