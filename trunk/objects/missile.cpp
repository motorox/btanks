
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

#include "object.h"
#include "resource_manager.h"
#include "config.h"

class Missile : public Object {
public:
	std::string type;
	Missile(const std::string &type) : Object("missile"), type(type) {}
	virtual void calculate(const float dt);
	virtual Object * clone() const;
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
	void onSpawn();

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(type);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(type);
	}

};

void Missile::onSpawn() {
	play("main", true);
	Object *_fire = spawnGrouped("single-pose", "missile-fire", v3<float>::empty, Centered);
	_fire->impassability = 0;
	add("fire", _fire);
	
	_velocity.normalize();
	int dir = _velocity.getDirection16();
	if (dir) {
		setDirection(dir - 1);
	}
}

void Missile::calculate(const float dt) {
	if (type == "guided") {
		std::vector<std::string> targets;
		targets.push_back("player");
		targets.push_back("kamikaze");
	
		v3<float> pos, vel;
	
		if (getNearest(targets, pos, vel)) {
			_velocity = pos;
		}

		GET_CONFIG_VALUE("objects.guided-missile.rotation-time", float, rotation_time, 0.2);
		limitRotation(dt, 16, rotation_time, false, false);
	}
}

void Missile::emit(const std::string &event, BaseObject * emitter) {
	if (event == "collision") {
		v3<float> dpos = getRelativePos(emitter) / 2;
		dpos.z = 0;
		emit("death", emitter);
	} if (event == "death" && type == "smoke") {
		spawn("smoke-cloud", "smoke-cloud");
		Object::emit(event, emitter);
	} else if (event == "death" && type == "nuke") {
		spawn("nuclear-explosion", "nuclear-explosion");
		Object::emit(event, emitter);
	} else if (event == "death") {
		spawn("explosion", "missile-explosion");
		Object::emit(event, emitter);
	} else Object::emit(event, emitter);
}


Object* Missile::clone() const  {
	return new Missile(*this);
}

REGISTER_OBJECT("guided-missile", Missile, ("guided"));
REGISTER_OBJECT("dumb-missile", Missile, ("dumb"));
REGISTER_OBJECT("smoke-missile", Missile, ("smoke"));
REGISTER_OBJECT("nuke-missile", Missile, ("nuke"));
REGISTER_OBJECT("boomerang-missile", Missile, ("boomerang"));
REGISTER_OBJECT("stun-missile", Missile, ("boomerang"));
