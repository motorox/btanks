/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
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

#include "menu.h"
#include "textitem.h"
#include "menuitem.h"
#include "mapitem.h"
#include "vehicleitem.h"
#include "game.h"
#include "start_server_menu.h"
#include "join_server_menu.h"

#include "sdlx/surface.h"
#include "sdlx/font.h"
#include "sdlx/color.h"
#include "mrt/logger.h"
#include "mrt/exception.h"
#include "config.h"
#include "resource_manager.h"
#include "menu_config.h"

#define ITEM_SPACING 10

MainMenu::MainMenu(const int w, const int h) : _active_item(0) {
	MenuConfig->load();
	
	_active = true;
	
	LOG_DEBUG(("loading font..."));
	_font = ResourceManager->loadFont("big", false);

	LOG_DEBUG(("loading background..."));
	_background.init("menu/background_box.png", "menu/highlight_big.png", 407, 338);
	_background_area.w = _background.w;
	_background_area.h = _background.h;
	
	LOG_DEBUG(("creating menu..."));
	_active_item = 0;
	_active_menu.clear();
	
	std::string address;
	Config->get("multiplayer.recent-host", address, "LOCALHOST");
#ifndef RELEASE
	_items[""].push_back(new MenuItem(_font, "start-game", "submenu", "START GAME"));
#endif
	_items[""].push_back(new MenuItem(_font, "multiplayer", "submenu", "OLD MULTIPLAYER MENU"));
	_items[""].push_back(new MenuItem(_font, "#start-server", "submenu", "START SERVER"));
	_items[""].push_back(new MenuItem(_font, "#join-server", "submenu", "JOIN GAME"));
	_items[""].push_back(new MenuItem(_font, "options", "submenu", "OPTIONS"));
	_items[""].push_back(new MenuItem(_font, "credits", "command", "CREDITS"));
	_items[""].push_back(new MenuItem(_font, "quit", "command", "QUIT"));

#ifndef RELEASE
	_items["start-game"].push_back(new MenuItem(_font, "start", "command", "USE TANK", "tank"));
	_items["start-game"].push_back(new MenuItem(_font, "start", "command", "USE LAUNCHER", "launcher"));
	_items["start-game"].push_back(new MenuItem(_font, "start", "command", "USE SHILKA", "shilka"));
	_items["start-game"].push_back(new MenuItem(_font, "back", "back", "BACK"));

	//_items["options"].push_back(new MenuItem(_font, "player1", "iterable", "PLAYER 1 CONTROL: AI"));
	
	_items["options"].push_back(new MenuItem(_font, "back", "back", "BACK"));
#endif

	_items["multiplayer"].push_back(new MenuItem(_font, "m-start", "command", "START NEW GAME"));
	_items["multiplayer"].push_back(new MenuItem(_font, "multiplayer-join", "submenu", "JOIN GAME"));
	_items["multiplayer"].push_back(new MenuItem(_font, "s-start", "command", "SPLIT SCREEN GAME"));
	_items["multiplayer"].push_back(new MapItem (_font, "map"));
	_items["multiplayer"].push_back(new VehicleItem(_font, "vehicle 1", "1"));
	_items["multiplayer"].push_back(new VehicleItem(_font, "vehicle 2", "2"));
	_items["multiplayer"].push_back(new MenuItem(_font, "back", "back", "BACK"));

	_items["multiplayer-join"].push_back(new TextItem(_font, "multiplayer.recent-host", "address", address));
	_items["multiplayer-join"].push_back(new MenuItem(_font, "m-join", "command", "JOIN GAME"));
	//_items["multiplayer-join"].push_back(new MenuItem(_font, "port", "text", "9876"));
	_items["multiplayer-join"].push_back(new MenuItem(_font, "back", "back", "BACK"));

	_items[_active_menu][_active_item]->onFocus();
	
	_special_menus["#start-server"] = new StartServerMenu(this, w, h);
	_special_menus["#join-server"] = new JoinServerMenu(this, w, h);

	recalculateSizes();

	Game->key_signal.connect(sigc::mem_fun(this, &MainMenu::onKey));
	Game->mouse_signal.connect(sigc::mem_fun(this, &MainMenu::onMouse));
}

void MainMenu::recalculateSizes() {
	_menu_size.x = _menu_size.y = 0;
	for(ItemList::const_iterator i = _items[_active_menu].begin(); i != _items[_active_menu].end(); ++i) {
		int w, h;
		(*i)->getSize(w, h);
		if (w > _menu_size.x) 
			_menu_size.x = w;
		_menu_size.y += h + ITEM_SPACING;
	}
}

void MainMenu::tick(const float dt) {
	if (!_active)
		return;
	
/*	for(std::map<const std::string, BaseMenu *>::iterator i = _special_menus.begin(); i != _special_menus.end(); ++i) {
		if (i->second)
			i->second->tick(dt);
	}
*/
	std::map<const std::string, BaseMenu *>::iterator i = _special_menus.find(_active_menu);
	if (i != _special_menus.end() && i->second != NULL)
		i->second->tick(dt);
}


void MainMenu::deinit() {
	for(MenuMap::iterator m = _items.begin(); m != _items.end(); ++m) {
		for(ItemList::iterator i = m->second.begin(); i != m->second.end(); ++i) {
			delete *i;
			*i = NULL;
		}
	}
	_items.clear();
	for(std::map<const std::string, BaseMenu *>::iterator i = _special_menus.begin(); i != _special_menus.end(); ++i) {
		delete i->second;
	}
	_special_menus.clear();
	_menu_path.clear();
	_active_menu.clear();
	_active_item = 0;
	
	MenuConfig->save();
}

MainMenu::~MainMenu() { 
	LOG_DEBUG(("cleaning up menus..."));
	deinit(); 
}


bool MainMenu::onKey(const SDL_keysym sym) {
	if (!_active)
		return false;
		
	BaseMenu * bm = getMenu(_active_menu);
	if (bm != NULL) {
		return bm->onKey(sym);
	}
	
	if (_items[_active_menu].empty())
		throw_ex(("no menu '%s' found", _active_menu.c_str()));
	MenuItem * item = _items[_active_menu][_active_item];
	if (item->onKey(sym))
		return true;
	
	switch(sym.sym) {
		case SDLK_UP:
			_items[_active_menu][_active_item]->onLeave();

			if (_active_item == 0) 
				_active_item = _items[_active_menu].size() - 1;
			else --_active_item;
			_items[_active_menu][_active_item]->onFocus();
			return true;

		case SDLK_DOWN:
			_items[_active_menu][_active_item]->onLeave();
			if (_active_item == _items[_active_menu].size() - 1) 
				_active_item = 0;
			else ++_active_item;
			_items[_active_menu][_active_item]->onFocus();
			return true;

		case SDLK_RETURN: {
				
				const std::string &name = item->name;
				if (item->type == "submenu") {
					LOG_DEBUG(("entering submenu '%s'", name.c_str()));
					if (name[0] == '#') {
						_menu_path.push_front(MenuID(_active_item, _active_menu));
						_active_menu = name;
						return true;
					}
					if (_items[name].empty())
						throw_ex(("no submenu %s found or it's empty", name.c_str()));
					_menu_path.push_front(MenuID(_active_item, _active_menu));
					_items[_active_menu][_active_item]->onLeave();
					_active_menu = name;
					_active_item = 0;
					_items[_active_menu][_active_item]->onFocus();
					recalculateSizes();
				} else if (item->type == "back") {
					if (!back()) 
						throw_ex(("cannot do 'back' command from top-level menu"));
				} else if (item->type == "command") {
					LOG_DEBUG(("command: %s", name.c_str()));
					menu_signal.emit(name, item->getValue());
				} else if (item->type == "iterable") {
					item->onClick();
					recalculateSizes();
				} else if (item->type == "text") {
					item->onClick();
				} else throw_ex(("unknown menu item type: %s", item->type.c_str()));
			}
			return true;
		case SDLK_ESCAPE: 
			return false;
		default: 
			break;
	}
	//LOG_DEBUG(("active item = %u", _active_item));
	return false;
}

#include "resource_manager.h"

void MainMenu::render(sdlx::Surface &dst) {
	if (!_active)
		return;
		
	BaseMenu * sm = getMenu(_active_menu);
	if (sm != NULL) {
		sm->render(dst, 0, 0);
		return;
	}

	int base_x = (dst.getWidth() - _background.w) / 2, base_y = (dst.getHeight() - _background.h) / 2;
	_background.render(dst, base_x, base_y);
	
	int x = (dst.getWidth() - _menu_size.x) /2;
	int y = (dst.getHeight() - _menu_size.y) / 2;

	_background_area.x = x;
	_background_area.y = y;
	
	const ItemList & items = _items[_active_menu];
	size_t n = items.size();
	for(size_t i = 0; i < n ;++i) {
		int w,h;
		items[i]->getSize(w, h);

		if (_active_item == i) {
			_background.renderHL(dst, base_x, y + h / 2);
		}
	
		items[i]->render(dst, x + (_menu_size.x - w) / 2, y);
		y += h + ITEM_SPACING;
	}
}

void MainMenu::setActive(const bool a) {
	_active = a;
}

void MainMenu::reset() {
	_items[_active_menu][_active_item]->onLeave();
	_menu_path.clear();
	_active_menu.clear();
	_active_item = 0;
	_items[_active_menu][_active_item]->onFocus();
	recalculateSizes();
}

const bool MainMenu::back() {
	if (_menu_path.size() == 0) 
		return false;
	
	if (_active_menu[0] != '#')
		_items[_active_menu][_active_item]->onLeave();
	
	_active_item = _menu_path.front().first;
	_active_menu = _menu_path.front().second;
	
	_menu_path.pop_front();
	
	if (_active_menu[0] != '#')
		_items[_active_menu][_active_item]->onFocus();
	
	recalculateSizes();
	return true;
}

bool MainMenu::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (!_active || !pressed)
		return false;
	
	BaseMenu * bm = getMenu(_active_menu);
	if (bm != NULL) {
		return bm->onMouse(button, pressed, x, y);
	}
	if (_background_area.in(x, y)) {
		sdlx::Rect item_area = _background_area;
		const ItemList & items = _items[_active_menu];
		for(size_t i = 0; i < items.size() ;++i) {
			int w, h;
			items[i]->getSize(w, h);
			item_area.h = h;
			
			if (item_area.in(x, y)) {
				_active_item = i;
				LOG_DEBUG(("clicked item %d", i));
				return true;
			}

			item_area.y += h + ITEM_SPACING;
		}
		return false;
	}
	//LOG_DEBUG(("%d %c %d %d", button, pressed?'+':'-', x, y));
	return false;
}

BaseMenu *MainMenu::getMenu(const std::string &menu) {
	return _special_menus[menu];
}
