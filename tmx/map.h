#ifndef __BT_MAP_H__
#define __BT_MAP_H__

#include <map>
#include <string>
#include <stack>
#include "mrt/xml.h"
#include "mrt/chunk.h"
#include "math/v3.h"
#include "math/matrix.h"
#include "mrt/singleton.h"

namespace sdlx {
class Surface;
class Rect;
}

class TMXEntity;
class Layer;
class Object;

class IMap : protected mrt::XMLParser {
public:
	DECLARE_SINGLETON(IMap);
	
	typedef std::map<const std::string, std::string> PropertyMap;
	PropertyMap properties;

	~IMap();
	void clear();
	void load(const std::string &name);
	const std::string & getName() const { return _name; }
	const bool loaded() const;
	
	
	void render(sdlx::Surface &window, const sdlx::Rect &dst, const int z1, const int z2) const;
	const v3<int> getSize() const;
	const v3<int> getTileSize() const;
	
	virtual const int getImpassability(const Object *obj, const sdlx::Surface &object_surf, const v3<int>& pos, v3<int> *tile_pos = NULL) const;
	void getImpassabilityMatrix(Matrix<int> &matrix) const { matrix = _imp_map; }
	static const int pathfinding_step;

private:
	virtual void start(const std::string &name, Attrs &attr);
	virtual void end(const std::string &name);
	virtual void charData(const std::string &data);

	Matrix<int> _imp_map;
	inline const bool collides(const sdlx::Surface&, const int dx, const int dy, const sdlx::Surface *tile) const;

	int _w, _h, _tw, _th, _firstgid;
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

SINGLETON(Map, IMap);

#endif

