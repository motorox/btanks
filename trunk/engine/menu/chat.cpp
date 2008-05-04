#include "chat.h"
#include "resource_manager.h"
#include "menu/text_control.h"
#include "sdlx/font.h"
#include "player_slot.h"
#include "team.h"

Chat::Chat(const size_t lines) :nick_w(0), lines(lines) {
	_font[0] = ResourceManager->loadFont("small", true);
	for(int t = 0; t < 4; ++t)
		_font[t + 1] = ResourceManager->loadFont(mrt::formatString("small_%s", Team::get_color((Team::ID)t)), true);
	 
	add(4, 0, _input = new TextControl("small"));
}

void Chat::clear() {
	text.clear();
	nick_w = 0;
	_input->set(std::string());
	last_message.clear();
	hide();
	layout();
}

void Chat::render(sdlx::Surface &surface, const int x, const int y) const {
	int ybase = 0;
	for(Text::const_iterator i = text.begin(); i != text.end(); ++i) {
		const Line &line = *i;
		line.font->render(surface, x + 4, y + ybase, line.nick);
		line.font->render(surface, x + 4 + nick_w, y + ybase, line.message);
		ybase += line.font->getHeight();
	}
	if (!hidden())
		Container::render(surface, x, y);
}

void Chat::layout() {
	int xp = 4;
	int yp = 0;
	for(Text::const_iterator i = text.begin(); i != text.end(); ++i) {
		const Line &line = *i;
		yp += line.font->getHeight();
	}
	setBase(_input, xp, yp);
}


void Chat::addMessage(const PlayerSlot &slot, const std::string &m) {
	//LOG_DEBUG(("addMessage('%s', '%s')", nick.c_str(), m.c_str()));
	const std::string n = "<" + slot.name + ">";
	Line line(n, m, _font[0]);
	text.push_back(line);
	
	if (text.size() > lines)
		text.erase(text.begin());
	
	size_t nw = line.font->render(NULL, 0, 0, n);
	if (nw > nick_w)
		nick_w = nw;
	layout();
}


bool Chat::onKey(const SDL_keysym sym) {
	switch(sym.sym) {
	case SDLK_KP_ENTER:
	case SDLK_RETURN: 
		last_message = _input->get();
					
	case SDLK_ESCAPE: 
		if (sym.sym == SDLK_ESCAPE) 
			last_message.clear();
		
		_input->set(std::string());
		invalidate(true);
		return true;
	default: 
		Container::onKey(sym);
	}
	return true;
}

