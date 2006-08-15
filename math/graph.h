#ifndef __BTANKS_GRAPH_H__
#define __BTANKS_GRAPH_H__

#include <map>
#include <limits>
#include "mrt/exception.h"

template <class T> class Graph {
public:
	void insert(int a, int b, const T w) {
		if (a > b) {
			int c = b; b = a; a = c;
		} else if (a == b) {
			throw_ex(("inserting edge (%d, %d) in graph (same vertexes)", a, b));
		}
		_edges[EdgeMap::key_type(a, b)] = w;
	}
	const T get(int a, int b) const {
		if (a > b) {
			int c = b; b = a; a = c;
		} else if (a == b) {
			throw_ex(("getting edge (%d, %d) from graph (same vertexes)", a, b));
		}
		typename EdgeMap::const_iterator i = _edges.find(EdgeMap::key_type(a, b));
		if (i == _edges.end())
			return std::numeric_limits<T>::infinity();
		return i->second;
	}
private:
	typedef std::map<const std::pair<int, int> , T> EdgeMap;
	EdgeMap _edges;
};

#endif
