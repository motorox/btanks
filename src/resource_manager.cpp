#include "resource_manager.h"
#include "mrt/logger.h"
#include "sdlx/surface.h"
#include "animated_object.h"
#include "animation_model.h"

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

		long tw = atol(attr["tile_width"].c_str());
		long th = atol(attr["tile_height"].c_str());
		if (tw == 0) tw = _tw;
		if (th == 0) th = _th;

		float speed = atof(attr["speed"].c_str());
		if (speed == 0) speed = 1;

		sdlx::Surface *s = NULL;
		TRY { 
			s = new sdlx::Surface;
			const std::string fname = "data/tiles/" + attr["tile"];
			s->loadImage(fname);
			LOG_DEBUG(("loaded animation '%s' from '%s'", id.c_str(), fname.c_str()));
			_animations[id] = new AnimatedObject(s, tw, th, speed);
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
	//LOG_DEBUG(("IResourceManager ctor"));
}

AnimatedObject *IResourceManager::getAnimation(const std::string &id) {
	AnimationMap::iterator i;
	if ((i = _animations.find(id)) == _animations.end()) 
		throw_ex(("could not find animation with id '%s'", id.c_str()));
	return i->second;
}

AnimationModel *IResourceManager::getAnimationModel(const std::string &id) {
	AnimationModelMap::iterator i;
	if ((i = _animation_models.find(id)) == _animation_models.end()) 
		throw_ex(("could not find animation with id '%s'", id.c_str()));
	return i->second;
}



void IResourceManager::init(const std::string &fname) {
	LOG_DEBUG(("loading resources from file: %s", fname.c_str()));
	parseFile(fname);
}


void IResourceManager::clear() {
	LOG_DEBUG(("freeing resources"));
	for(AnimationMap::iterator i = _animations.begin(); i != _animations.end(); ++i) {
		delete i->second;
		i->second = NULL;
	}
	_animations.clear();
	_am = NULL;
}

IResourceManager::~IResourceManager() {
	clear();
}

