#include "resource_manager.h"
#include "mrt/logger.h"
#include "sdlx/surface.h"
#include "animated_object.h"
#include "animation_model.h"
#include "utils.h"

#include <algorithm>

IMPLEMENT_SINGLETON(ResourceManager, IResourceManager)


void IResourceManager::start(const std::string &name, Attrs &attr) {	
	if (name == "resources") {
		_tw = atol(attr["tile_width"].c_str());
		if (_tw == 0)
			throw_ex(("resources tag must contain `tile_width' attribute (default tile width)"));
		_th = atol(attr["tile_height"].c_str());
		if (_th == 0)
			throw_ex(("resources tag must contain `tile_height' attribute (default tile height)"));
		if (attr["version"].size() == 0)
			throw_ex(("resources tag must contain `version' attribute, now only 0.3 supported"));
		LOG_DEBUG(("file version: %s", attr["version"].c_str()));
	} else if (name == "animation") {
		const std::string &id = attr["id"];
		if (id.size() == 0)
			throw_ex(("animation.id was not set"));

		const std::string &model = attr["model"];
		if (model.size() == 0)
			throw_ex(("animation.model was not set"));

		long tw = atol(attr["tile_width"].c_str());
		long th = atol(attr["tile_height"].c_str());
		if (tw == 0) tw = _tw;
		if (th == 0) th = _th;

		sdlx::Surface *s = NULL;
		TRY { 
			s = new sdlx::Surface;
			const std::string fname = "data/tiles/" + attr["tile"];
			s->loadImage(fname);
			s->convertAlpha();
			_surfaces[attr["tile"]] = s;
			s = NULL;
			
			LOG_DEBUG(("loaded animation '%s' from '%s'", id.c_str(), fname.c_str()));
			_animations[id] = new Object(id);
			_animations[id]->init(model, attr["tile"], tw, th);
		} CATCH("animation", { delete s; s = NULL; });
	} else if (name == "animation-model") {
		const std::string & id = attr["id"];
		if (id.size() == 0) 
			throw_ex(("animation model must have id"));
		
		float speed = atof(attr["speed"].c_str());
		if (speed == 0)
			throw_ex(("animation model must have default speed"));
		
		_am = new AnimationModel(speed);
		_am_id = id;		
	} else if (name == "pose") {
		_pose_id = attr["id"];
		if (_pose_id.size() == 0) 
			throw_ex(("pose must have id"));
			
		float speed = atof(attr["speed"].c_str());
		if (speed == 0)
			speed = _am->default_speed;
		_pose = new Pose(speed);
		//nope
	} else LOG_WARN(("unhandled tag: %s", name.c_str()));
}

void IResourceManager::end(const std::string &name) {
	if (name == "pose") {
		LOG_DEBUG(("pose frames: %s", _data.c_str()));
		std::vector<std::string> frames;
		mrt::split(frames, _data, ",");
		
		for(size_t i = 0; i < frames.size(); ++i) {
			//LOG_DEBUG(("%d: %s", i, frames[i].c_str()));
			mrt::trim(frames[i]);
			unsigned int frame = atoi(frames[i].c_str());
			//LOG_DEBUG(("%d: %d", i, frame));
			_pose->frames.push_back(frame);
		}
		_am->addPose(_pose_id, _pose);
		_pose = NULL;
	} else if (name == "animation-model") {
		LOG_DEBUG(("adding animation model '%s'", _am_id.c_str()));
		delete _animation_models[_am_id];
		_animation_models[_am_id] = _am;
		_am = NULL;
	}
}
void IResourceManager::charData(const std::string &data) {
	_data = data;
}

IResourceManager::IResourceManager() : _am(0) {
}


Object *IResourceManager::getAnimation(const std::string &id) {
	AnimationMap::iterator i;
	if ((i = _animations.find(id)) == _animations.end()) 
		throw_ex(("could not find animation with id '%s'", id.c_str()));
	return i->second;
}

const Object *IResourceManager::getAnimation(const std::string &id) const {
	AnimationMap::const_iterator i;
	if ((i = _animations.find(id)) == _animations.end()) 
		throw_ex(("could not find animation with id '%s'", id.c_str()));
	return i->second;
}

Object *IResourceManager::createAnimation(const std::string &id) {
	return new Object(*getAnimation(id));	
}

AnimationModel *IResourceManager::getAnimationModel(const std::string &id) {
	AnimationModelMap::iterator i;
	if ((i = _animation_models.find(id)) == _animation_models.end()) 
		throw_ex(("could not find animation with id '%s'", id.c_str()));
	return i->second;
}

const sdlx::Surface *IResourceManager::getSurface(const std::string &id) const  {
	SurfaceMap::const_iterator i;
	if ((i = _surfaces.find(id)) == _surfaces.end()) 
		throw_ex(("could not find surface with id '%s'", id.c_str()));
	return i->second;
}


void IResourceManager::init(const std::string &fname) {
	LOG_DEBUG(("loading resources from file: %s", fname.c_str()));
	parseFile(fname);
}

void IResourceManager::initMe(Object *o, const std::string &animation) const {
	const std::string classname = o->classname;
	o->init(*getAnimation(animation));
	o->classname = classname;
}

void IResourceManager::clear() {
	LOG_DEBUG(("freeing resources"));
	std::for_each(_animations.begin(), _animations.end(), delete_ptr2<AnimationMap::value_type>());
	_animations.clear();
	std::for_each(_animation_models.begin(), _animation_models.end(), delete_ptr2<AnimationModelMap::value_type>());
	_animation_models.clear();
	std::for_each(_surfaces.begin(), _surfaces.end(), delete_ptr2<SurfaceMap::value_type>());
	_surfaces.clear();
	std::for_each(_objects.begin(), _objects.end(), delete_ptr2<ObjectMap::value_type>());
	_objects.clear();

	_am = NULL;
}

IResourceManager::~IResourceManager() {
	clear();
}

void IResourceManager::registerObject(const std::string &classname, Object *o) {
	delete _objects[classname];
	_objects[classname] = o;
	LOG_DEBUG(("classname %s registered at %p", classname.c_str(), (void*)o));
}

Object *IResourceManager::createObject(const std::string &classname, const std::string &animation) const {
	ObjectMap::const_iterator i = _objects.find(classname);
	if (i == _objects.end())
		throw_ex(("classname '%s' was not registered", classname.c_str()));
	return i->second->clone(animation);
}
