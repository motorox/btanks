#include "object.h"
#include "resource_manager.h"

class Bullet : public Object {
public:
	Bullet(const std::string &type) : Object("bullet"), _type(type) {}
	virtual void calculate(const float dt);
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
private: 
	std::string _type;
};


void Bullet::calculate(const float dt) {
	_velocity.normalize();
	
	int dir = _velocity.getDirection8();
	if (dir) {
		setDirection(dir - 1);
	}
	_velocity.quantize8();
}

void Bullet::onSpawn() {
	play("move", true);
}

void Bullet::emit(const std::string &event, BaseObject * emitter) {
	if (event == "collision" || event == "death") {
		if (_type == "regular" && event == "collision") 
			spawn("explosion", "explosion", v3<float>(0,0,1), v3<float>(0,0,0));
		if (_type == "dirt") 
			spawn("dirt", "dirt", v3<float>(0,0,0.1), v3<float>(0,0,0));
		Object::emit("death", emitter);
	} else Object::emit(event, emitter);
}


Object* Bullet::clone() const  {
	Object *a = new Bullet(*this);
	a->setDirection(getDirection());
	return a;
}

REGISTER_OBJECT("bullet", Bullet, ("regular"));
REGISTER_OBJECT("dirt-bullet", Bullet, ("dirt"));
