#include "object.h"
#include "context.h"

using namespace clunk;

Object::Object(Context *context) : context(context) {}

void Object::updatePV(const v3<float> &pos, const v3<float> &vel) {
	this->pos = pos;
	this->vel = vel;
}

void Object::add(Source *source) {
	sources.insert(source);
}

void Object::remove(Source *source) {
	sources.erase(source);
}

void Object::remove_all() {
	sources.clear();
}

Object::~Object() {
	context->delete_object(this);
}
