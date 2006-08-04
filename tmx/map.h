#ifndef __BT_MAP_H__
#define __BT_MAP_H__

#include <map>
#include <string>
#include <stack>
#include "mrt/xml.h"
#include "mrt/chunk.h"
#include "world_map.h"

namespace sdlx {
class Surface;
class Rect;
}

class TMXEntity;
class Layer;
class Object;

class Map : public WorldMap, protected mrt::XMLParser {
public:
	typedef std::map<const std::string, std::string> PropertyMap;
	PropertyMap properties;

	~Map();
	void clear();
	void load(const std::string &name);
	const std::string & getName() const { return _name; }
	const bool loaded() const;
	
	virtual void start(const std::string &name, Attrs &attr);
	virtual void end(const std::string &name);
	virtual void charData(const std::string &data);
	
	void render(sdlx::Surface &window, const sdlx::Rect &dst, const int z1, const int z2);
	const sdlx::Rect getSize() const;
	
	virtual const int getImpassability(const sdlx::Surface &object_surf, const v3<int>& pos) const;
private:
	const bool collides(const sdlx::Surface&, const int dx, const int dy, const unsigned tid) const;

	long _w, _h, _tw, _th, _firstgid;
	int _lastz;
	mrt::Chunk _data;
	sdlx::Surface *_image;
	bool _image_is_tileset;

	PropertyMap _properties;
	
	typedef std::map<const int, Layer *> LayerMap;
	LayerMap _layers;
	bool layer;

	typedef std::vector<sdlx::Surface *> TileMap;
	TileMap _tiles;

	struct Entity {
		mrt::XMLParser::Attrs attrs;
		std::string data;
		Entity(const mrt::XMLParser::Attrs & attrs) : attrs(attrs) {}
	};
	
	typedef std::stack<Entity> EntityStack;
	EntityStack _stack;
	
	std::string _name;
};

#endif

