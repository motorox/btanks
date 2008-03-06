#ifndef BTANKS_MENU_SCROLL_LIST_H__
#define BTANKS_MENU_SCROLL_LIST_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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

#include <deque>
#include <string>
#include "container.h"
#include "box.h"
#include "sdlx/font.h"
#include "sdlx/rect.h"
#include "export_btanks.h"

class BTANKSAPI ScrollList : public Container {
public: 
	ScrollList(const std::string &background, const std::string &font, const int w, const int h, const int spacing = 3, const int hl_h = 24);
	
	void initBG(const std::string &background, const int w, const int h, const int hl_h = 0);
	
	virtual void clear();
	virtual void append(const std::string &item);
	virtual void append(Control *control);
	
	const int size() const { return _list.size(); }
	const bool empty() const { return _list.empty(); }
	
	Control * getItem(const int idx) { return _list[idx]; }
	const Control * getItem(const int idx) const { return _list[idx]; }

	const int get() const { return _current_item; }
	const std::string getValue() const;
	void set(const int idx);
	void remove(const int idx);
	
	virtual void tick(const float dt);
	virtual void render(sdlx::Surface &surface, const int x, const int y) const;
	virtual bool onKey(const SDL_keysym sym);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	virtual void getSize(int &w, int &h) const;

	void getItemY(const int idx, int &ypos, int &height) const;
	const int getItemIndex(const int yp) const;
	
	void sort();
	
	~ScrollList();
	
	void up(const int n = 1);
	void down(const int n = 1);

	enum Align { AlignLeft, AlignCenter, AlignRight };
	void setAlign(const Align align) { _align = align; }

private:
	Box _background;
	const sdlx::Surface *_scrollers;
	mutable sdlx::Rect _up_area, _down_area, _items_area;
	mutable int _client_w, _client_h;
	Align _align;

	float _pos, _vel;
protected:
	const sdlx::Font *_font;
	typedef std::deque<Control *> List;
	List _list;
	int _current_item;
	int _spacing;
};

#endif