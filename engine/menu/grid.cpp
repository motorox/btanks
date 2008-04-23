#include "grid.h"
#include "sdlx/rect.h"

Grid::Grid(const int w, const int h) : _spacing(0) {
	_controls.resize(h);
	for(int i = 0; i < h; ++i) {
		_controls[i].resize(w);
	}
	_split_w.resize(w);
	_split_h.resize(h);
}

Grid::~Grid() {
	for(size_t i = 0; i < _controls.size(); ++i) {
		for(size_t j = 0; j < _controls[i].size(); ++j) {
			delete _controls[i][j].c;
		}
	}

}

void Grid::set(const int r, const int c, Control *ctrl, const int align) {
	if (r < 0 || r >= (int)_controls.size())
		throw_ex(("set(%d, %d) is out of range", r, c));
	Row &row = _controls[r];
	if (c < 0 || c >= (int) row.size())
		throw_ex(("set(%d, %d) is out of range", r, c));

	ControlDescriptor &d = row[c];
	delete d.c;
	d.c = ctrl;
	d.align = align;
}

void Grid::render(sdlx::Surface &surface, const int x, const int y) const {
	int yp = y;
	for(size_t i = 0; i < _controls.size(); ++i) {
		int xp = x;
		const Row &row = _controls[i];
		for(size_t j = 0; j < row.size(); ++j) {
			const ControlDescriptor &d = row[j];
			if (d.c != NULL) {
				int xc, yc;
				int cw, ch;
				d.c->getSize(cw, ch);

				if (d.align & Center) {
					xc = (_split_w[j] - cw) / 2;
				} else if (d.align & Right) {
					xc = _split_w[j] - cw - _spacing;
				} else {
					xc = _spacing;
				}

				if (d.align & Middle) {
					yc = (_split_h[i] - ch) / 2;
				} else if (d.align & Bottom) {
					yc = _split_h[i] - ch - _spacing;
				} else {
					yc = _spacing;
				}

				d.c->render(surface, xp + xc, yp + yc);
			}
			xp += _split_w[j];
		}
		yp += _split_h[i];
	}
}

Grid::ControlDescriptor * Grid::find(const int x, const int y) {
	int yp = 0;
	for(size_t i = 0; i < _controls.size(); ++i) {
		if (yp > y)
			return NULL;
		
		int xp = 0;
		Row &row = _controls[i];
		for(size_t j = 0; j < row.size(); ++j) {
			if (xp > x)
				break;
			
			ControlDescriptor &d = row[j];
			if (d.c != NULL) {
				int xc, yc;
				int cw = -1, ch = -1;
				d.c->getSize(cw, ch);
				assert(cw >= 0 && ch >= 0);

				if (d.align & Center) {
					xc = (_split_w[j] - cw) / 2;
				} else if (d.align & Right) {
					xc = _split_w[j] - cw - _spacing;
				} else {
					xc = _spacing;
				}

				if (d.align & Middle) {
					yc = (_split_h[i] - ch) / 2;
				} else if (d.align & Bottom) {
					yc = _split_h[i] - ch - _spacing;
				} else {
					yc = _spacing;
				}
				
				//LOG_DEBUG(("%u,%u: x: %d y: %d, xp: %d, yp: %d, xc: %d, yc: %d, cw: %d, ch: %d", i, j, x, y, xp, yp, xc, yc, cw, ch));
				sdlx::Rect rect(0, 0, cw, ch);
				if (rect.in(x - xp - xc, y - yp - yc))
					return &d;
			}
			xp += _split_w[j];
		}
		yp += _split_h[i];
	}
	return NULL;
}


void Grid::recalculate(const int w, const int h) {
	for(size_t i = 0; i < _split_w.size(); ++i) {
		_split_w[i] = 0;
	}
	for(size_t i = 0; i < _split_h.size(); ++i) {
		_split_h[i] = 0;
	}
	
	for(size_t i = 0; i < _controls.size(); ++i) {
		const Row &row = _controls[i];
		for(size_t j = 0; j < row.size(); ++j) {
			Control *c = row[j].c;
			if (c == NULL)
				continue;
			int cw = -1, ch = -1;
			c->getSize(cw, ch);
			assert(cw >= 0 && ch >= 0);
			
			cw += 2 * _spacing;
			ch += 2 * _spacing;
			if (cw > _split_w[j]) {
				_split_w[j] = cw;
			}
			if (ch > _split_h[i]) {
				_split_h[i] = ch;
			}
		}
	}

	if (w != 0) {
		int real_w = 0;
		for(size_t i = 0; i < _split_w.size(); ++i) 
			real_w += _split_w[i];
		int dx = (w - real_w) / (int)_split_w.size();
		for(size_t i = 0; i < _split_w.size(); ++i) 
			_split_w[i] += dx;
	}

	if (h != 0) {
		int real_h = 0;
		for(size_t j = 0; j < _split_h.size(); ++j) 
			real_h += _split_h[j];
		int dy = (h - real_h) / (int)_split_h.size();
		for(size_t i = 0; i < _split_h.size(); ++i) 
			_split_h[i] += dy;
	}
}

void Grid::getSize(int &w, int &h) const {
	w = h = 0;

	for(size_t i = 0; i < _split_w.size(); ++i) 
		w += _split_w[i];

	for(size_t j = 0; j < _split_h.size(); ++j) 
		h += _split_h[j];
}
	
bool Grid::onKey(const SDL_keysym sym) {
	for(size_t i = 0; i < _controls.size(); ++i) {
		Row &row = _controls[i];
		for(size_t j = 0; j < row.size(); ++j) {
			if (row[j].c != NULL &&	row[j].c->onKey(sym))
				return true;
		}
	}
	return false;
}

bool Grid::onMouse(const int button, const bool pressed, const int x, const int y) {
	//LOG_DEBUG(("%d, %d", x, y));
	ControlDescriptor * d = find(x, y);
	if (d == NULL || d->c == NULL)
		return false;
	return d->c->onMouse(button, pressed, x, y);
}

bool Grid::onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel) {
	ControlDescriptor * d = find(x, y);
	if (d == NULL || d->c == NULL)
		return false;
	return d->c->onMouseMotion(state, x, y, xrel, yrel);
}