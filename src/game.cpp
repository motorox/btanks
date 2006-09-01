#include "object.h"
#include "game.h"
#include "version.h"
#include "world.h"
#include "resource_manager.h"

#include "tmx/map.h"

#include "mrt/logger.h"
#include "mrt/exception.h"

#include "sdlx/system.h"
#include "sdlx/sdl_ex.h"
#include "sdlx/joystick.h"
#include "sdlx/ttf.h"
#include "sdlx/color.h"
#include "sdlx/fps.h"
#include "sdlx/net_ex.h"
#include "sdlx/tcp_socket.h"

#include "net/server.h"
#include "net/client.h"
#include "net/protocol.h"
#include "net/connection.h"

#include "SDL_gfx/SDL_gfxPrimitives.h"
#include <SDL/SDL_opengl.h>
#include <SDL/SDL_net.h>

#include "controls/joyplayer.h"
#include "controls/keyplayer.h"

#ifndef SDL_OPENGLBLIT
#define SDL_OPENGLBLIT 0
// using 0 as OPENGLBLIT value. SDL 1.3 or later
#endif


IMPLEMENT_SINGLETON(Game, IGame)

IGame::IGame() {
	LOG_DEBUG(("IGame ctor"));
}
IGame::~IGame() {}

const std::string IGame::data_dir = "data";

typedef void (APIENTRY *glEnable_Func)(GLenum cap);
typedef void (APIENTRY *glFlush_Func)(void);
typedef void (APIENTRY *glBlendFunc_Func) (GLenum sfactor, GLenum dfactor );

template <typename FuncPtr> union SharedPointer {
	FuncPtr call;
	void *ptr;
};

static SharedPointer<glEnable_Func> glEnable_ptr;
static SharedPointer<glBlendFunc_Func> glBlendFunc_ptr;
static SharedPointer<glFlush_Func> glFlush_ptr;

void IGame::init(const int argc, char *argv[]) {

	_server = NULL; _client = NULL;
#ifdef __linux__
//	putenv("SDL_VIDEODRIVER=dga");
#endif

	_opengl = false;
	bool fullscreen = false;
	bool dx = false;
	_vsync = true;
	int w = 800, h = 600;
	
	for(int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "--gl") == 0) _opengl = true;
		else if (strcmp(argv[i], "--fs") == 0) fullscreen = true;
		else if (strcmp(argv[i], "--no-vsync") == 0) _vsync = false;
#ifdef WIN32
		else if (strcmp(argv[i], "--dx") == 0) dx = true;
#endif
		else if (strcmp(argv[i], "-2") == 0) { w = 1024; h = 768; }
		else throw_ex(("unrecognized option: '%s'", argv[i]));
	}
	
	LOG_DEBUG(("gl: %s, vsync: %s, dx: %s", _opengl?"yes":"no", _vsync?"yes":"no", dx?"yes":"no"));
#ifdef WIN32
	_putenv("SDL_VIDEO_RENDERER=gdi");

	if (dx) 
#if SDL_MAJOR_VERSION >= 1 && SDL_MINOR_VERSION >= 3
		_putenv("SDL_VIDEO_RENDERER=d3d");
#else
		_putenv("SDL_VIDEODRIVER=directx");
#endif

#endif

//opengl renderer
#if SDL_MAJOR_VERSION >= 1 && SDL_MINOR_VERSION >= 3
	if (_opengl)
		_putenv("SDL_VIDEO_RENDERER=opengl");
#else
	if (_opengl)
		_putenv("SDL_VIDEODRIVER=opengl");
#endif

	LOG_DEBUG(("initializing SDL..."));
#ifdef DEBUG
	sdlx::System::init(SDL_INIT_EVERYTHING | SDL_INIT_NOPARACHUTE);
#else
	sdlx::System::init(SDL_INIT_EVERYTHING);
#endif

	if (_opengl) {
		LOG_DEBUG(("loading GL library"));
		if (SDL_GL_LoadLibrary(NULL) == -1) 
			throw_sdl(("SDL_GL_LoadLibrary"));

		glEnable_ptr.ptr = SDL_GL_GetProcAddress("glEnable");
		if (!glEnable_ptr.ptr)
			throw_ex(("cannot get address of glEnable"));
	
		glBlendFunc_ptr.ptr = SDL_GL_GetProcAddress("glBlendFunc");
		if (!glBlendFunc_ptr.ptr)
			throw_ex(("cannot get address of glBlendFunc"));

		glFlush_ptr.ptr = SDL_GL_GetProcAddress("glFlush");
		if (!glFlush_ptr.ptr)
			throw_ex(("cannot get address of glFlush"));
	}
	
	sdlx::Surface::setDefaultFlags(sdlx::Surface::Hardware | sdlx::Surface::Alpha | (_opengl? SDL_OPENGL: 0) );

	LOG_DEBUG(("initializing SDL_ttf..."));
	sdlx::TTF::init();

	LOG_DEBUG(("initializing SDL_net..."));
	if (SDLNet_Init() == -1) {
		throw_net(("SDLNet_Init"));
	}
	
	LOG_DEBUG(("probing for joysticks"));
	int jc = sdlx::Joystick::getCount();
	if (jc > 0) {
		LOG_DEBUG(("found %d joystick(s)", jc));
		//sdlx::Joystick::sendEvents(true);
		
		for(int i = 0; i < jc; ++i) {
			LOG_DEBUG(("%d: %s", i, sdlx::Joystick::getName(i).c_str()));
/*			sdlx::Joystick j;
			j.open(i);
			
			j.close();
*/
		}
	}
	if (_opengl) {
		if (_vsync) 
			SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
		SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
	
		glBlendFunc_ptr.call( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glEnable_ptr.call( GL_BLEND ) ;
	
		_window.setVideoMode(w, h, 32, SDL_HWSURFACE | SDL_OPENGL | SDL_OPENGLBLIT | (fullscreen?SDL_FULLSCREEN:0) | (_vsync?SDL_DOUBLEBUF:0));
	} else {
		_window.setVideoMode(w, h, 0, SDL_HWSURFACE | (_vsync?SDL_DOUBLEBUF:0) | SDL_SRCALPHA | (fullscreen?SDL_FULLSCREEN:0) | SDL_ANYFORMAT);
	}
	
	LOG_DEBUG(("created main surface. (%dx%dx%d, %s)", w, h, _window.getBPP(), ((_window.getFlags() & SDL_HWSURFACE) == SDL_HWSURFACE)?"hardware":"software"));

	sdlx::System::probeVideoMode();	
#if 0
	{
		SDL_Rect **modes;
		int i;

		/* Get available fullscreen/hardware modes */
		modes = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_HWSURFACE);

		/* Check is there are any modes available */
		if(modes == (SDL_Rect **)0) 
			throw_ex(("No video modes available"));
    
	    /* Check if our resolution is restricted */
    	if(modes == (SDL_Rect **)-1){
			LOG_DEBUG(("all resolutions available."));
		} else {
			/* Print valid modes */
			LOG_DEBUG(("available modes:"));
			for(i=0;modes[i];++i)
				LOG_DEBUG(("\t%dx%d", modes[i]->w, modes[i]->h));
		}
	}
#endif
	LOG_DEBUG(("setting caption..."));		
	SDL_WM_SetCaption(("Battle tanks - " + getVersion()).c_str(), "btanks");

	LOG_DEBUG(("initializing menus..."));		
	_main_menu.init(w, h);	

	_paused = false;
	_running = true;

	_window.update();
	
	LOG_DEBUG(("initializing resource manager..."));
	ResourceManager->init("data/resources.xml");

	LOG_DEBUG(("installing callbacks..."));
	key_signal.connect(sigc::mem_fun(this, &IGame::onKey));
	_main_menu.menu_signal.connect(sigc::mem_fun(this, &IGame::onMenu));
}

void IGame::onKey(const Uint8 type, const SDL_keysym key) {
	if (key.sym == SDLK_ESCAPE && type == SDL_KEYUP) {
		LOG_DEBUG(("escape hit, paused: %s", _paused?"true":"false"));
		_paused = !_paused;
		_main_menu.setActive(_paused);
	}
}

void IGame::onMenu(const std::string &name) {
	if (name == "quit") 
		_running = false;
	else if (name == "start") {
		LOG_DEBUG(("start single player requested"));
		loadMap("country");
		
		//_my_index = spawnPlayer("tank", "green-tank", "keys");
		_my_index = spawnPlayer("launcher", "green-launcher", "keys");
		spawnPlayer("ai-tank", "red-tank", "ai");
		//spawnPlayer("ai-player", "yellow-tank");
		//spawnPlayer("ai-player", "cyan-tank");
	} else if (name == "m-start") {
		LOG_DEBUG(("start multiplayer server requested"));
		loadMap("country");

		_my_index = spawnPlayer("tank", "green-tank", "keys");
		
		_server = new Server;
		_server->init(9876);
	} else if (name == "m-join") {
		std::string host = "localhost";
		unsigned port = 9876;
		TRY {
			_client = new Client;
			_client->init(host, port);
		} CATCH("_client.init", { delete _client; _client = NULL; return; });
		
		_main_menu.setActive(false);
	}
}


void IGame::loadMap(const std::string &name) {
	_main_menu.setActive(false);
	IMap &map = *IMap::get_instance();
	map.load(name);
	
	const v3<int> size = map.getSize();
	_players.clear();
	for (IMap::PropertyMap::iterator i = map.properties.begin(); i != map.properties.end(); ++i) {
		if (i->first.substr(0, 6) == "spawn:") {
			v3<int> pos;
			pos.fromString(i->second);
			if (pos.x < 0) 
				pos.x += size.x;
			if (pos.y < 0) 
				pos.y += size.y;
			LOG_DEBUG(("spawnpoint: %d,%d", pos.x, pos.y));
			
			PlayerSlot slot;
			slot.position = pos;
			_players.push_back(slot);
		}

		std::vector<std::string> res;
		mrt::split(res, i->first, ":");
		if (res.size() > 2 && res[0] == "object") {
			v3<int> pos;
			pos.fromString(i->second);
			//LOG_DEBUG(("object %s, animation %s, pos: %s", res[1].c_str(), res[2].c_str(), i->second.c_str()));
			World->addObject(ResourceManager->createObject(res[1], res[2]), pos.convert<float>());
		}
	}
}

const int IGame::spawnPlayer(const std::string &classname, const std::string &animation, const std::string &control_method) {
	size_t i, n = _players.size();
	for(i = 0; i < n; ++i) {
		if (_players[i].obj == NULL)
			break;
	}
	if (i == n) 
		throw_ex(("no available slots found from %d", n));
	PlayerSlot &slot = _players[i];

	delete slot.control_method;
	slot.control_method = NULL;
	
	if (control_method == "keys") {
		slot.control_method = new KeyPlayer(SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_SPACE);
	} else if (control_method != "ai" && control_method != "net") {
		throw_ex(("unknown control method '%s' used", control_method.c_str()));
	}
	LOG_DEBUG(("player: %s.%s using control method: %s", classname.c_str(), animation.c_str(), control_method.c_str()));
	Object *obj = ResourceManager->createObject(classname, animation);
	assert(obj != NULL);

	World->addObject(obj, slot.position.convert<float>());

	slot.obj = obj;
	return i;
}


void IGame::run() {
	LOG_DEBUG(("entering main loop"));
	SDL_Event event;
	IMap &map = *IMap::get_instance();

	sdlx::Rect window_size = _window.getSize();
	sdlx::Rect viewport = _window.getSize();
	sdlx::Rect passive_viewport;
	passive_viewport.w = passive_viewport.x = viewport.w / 3;
	passive_viewport.h = passive_viewport.y = viewport.h / 3;
	sdlx::Rect passive_viewport_stopzone(passive_viewport);
	
	{
		int xmargin = passive_viewport_stopzone.w / 4;
		int ymargin = passive_viewport_stopzone.h / 4;
		passive_viewport_stopzone.x += xmargin;
		passive_viewport_stopzone.y += ymargin;
		passive_viewport_stopzone.w -= 2*xmargin;
		passive_viewport_stopzone.h -= 2*ymargin;
	}
	
	float mapx = 0, mapy = 0, mapvx = 0, mapvy = 0;
	int fps_limit = 1000;
	
	float fr = fps_limit;
	int max_delay = 1000/fps_limit;
	LOG_DEBUG(("fps_limit set to %d, maximum frame delay: %d", fps_limit, max_delay));
	sdlx::Surface fps_s;
	fps_s.createRGB(32, 32, 32, SDL_SWSURFACE | SDL_SRCCOLORKEY);
	fps_s.convertToDisplay();
	
	if (SDL_SetColorKey(fps_s.getSDLSurface(), SDL_SRCCOLORKEY, 255) == -1)
		throw_sdl(("SDL_SetColorKey"));
	//fps_s.convertAlpha();

	while (_running) {
		Uint32 tstart = SDL_GetTicks();
		
		while (SDL_PollEvent(&event)) {
			switch(event.type) {
			case SDL_KEYDOWN:
#ifndef WIN32
				if (event.key.keysym.sym==SDLK_f && event.key.keysym.mod & KMOD_CTRL) {
					_window.toggleFullscreen();
					break;
				}
#endif
				if (event.key.keysym.sym==SDLK_s && event.key.keysym.mod & KMOD_CTRL) {
					_window.saveBMP("screenshot.bmp");
					break;
				}
			case SDL_KEYUP:
				key_signal.emit(event.key.type, event.key.keysym);
			break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				mouse_signal.emit(event.button.button, event.button.type, event.button.x, event.button.y);
				break;
		    case SDL_QUIT:
				_running = false;
			break;
    		}
		}
		
		const float dt = 1.0/fr;
		
		
		if (_running && !_paused) {
			{
				//updating all player states.
				for(std::vector<PlayerSlot>::iterator i = _players.begin(); i != _players.end(); ++i) {
					PlayerSlot &slot = *i;
					if (slot.control_method != NULL) {
						slot.control_method->updateState(slot.obj->getPlayerState());
					}
				}
			}
		
			World->tick(dt);
			
			if (_server) 
				_server->tick(dt);

			if (_client) 
				_client->tick(dt);
			
			if (_players.size()) {
				const Object * p = _players[_my_index].obj;
				v3<float> pos, vel;
				if (World->getInfo(p, pos, vel)) {
					//LOG_DEBUG(("player[0] %f, %f", vel.x, vel.y));
					int wx = (int)pos.x - viewport.x;
					int wy = (int)pos.y - viewport.y;
					if (passive_viewport_stopzone.in(wx, wy)) {
						mapvx = 0; 
						mapvy = 0;
					} else {
						mapvx = p->speed * 2 * (wx - passive_viewport.x) / passive_viewport.w ;
						mapvy = p->speed * 2 * (wy - passive_viewport.y) / passive_viewport.h ;
						/*
						LOG_DEBUG(("position : %f %f viewport: %d %d(passive:%d %d %d %d) mapv: %f %f", x, y,
							viewport.x, viewport.y, passive_viewport.x, passive_viewport.y, passive_viewport.w, passive_viewport.h, 
							mapvx, mapvy));
						*/
					}
				}
			}
		}
		Uint32 t_tick = SDL_GetTicks() - tstart;
		if (_opengl) {
			glFlush_ptr.call();
		}

		_window.fillRect(window_size, 0);
		map.render(_window, viewport, -1000, 0);
		World->render(_window, viewport);
		map.render(_window, viewport, 0, 1001);

		_main_menu.render(_window);
		
		
		std::string f = mrt::formatString("%d", (int)fr);
		fps_s.fillRect(fps_s.getSize(), 255);
		stringRGBA(fps_s.getSDLSurface(), 4, 4, f.c_str(), 0, 0, 0, 255);
		stringRGBA(fps_s.getSDLSurface(), 3, 3, f.c_str(), 255, 255, 255, 255);
		_window.copyFrom(fps_s, 0, 0);
		
		if (map.loaded()) {
			const v3<int> world_size = map.getSize();
		
			mapx += mapvx * dt;
			mapy += mapvy * dt;
			
			if (mapx < 0) 
				mapx = 0;
			if (mapx + viewport.w > world_size.x) 
				mapx = world_size.x - viewport.w;

			if (mapy < 0) 
				mapy = 0;
			if (mapy + viewport.h > world_size.y) 
				mapy = world_size.y - viewport.h;
			
			viewport.x = (Sint16) mapx;
			viewport.y = (Sint16) mapy;
			
			//LOG_DEBUG(("%f %f", mapx, mapy));
		}

		Uint32 t_render = SDL_GetTicks() - tstart;
		_window.flip();
		
		Uint32 t_flip = SDL_GetTicks() - tstart;
	
		int tdelta = SDL_GetTicks() - tstart;

		LOG_DEBUG(("tick time: %u, render time: %u, flip time: %u", t_tick, t_render, t_flip));
		if (tdelta < max_delay) {
			LOG_DEBUG(("tdelta: %d, delay: %d", tdelta, max_delay - tdelta));
			SDL_Delay(max_delay - tdelta);
		}

		tdelta = SDL_GetTicks() - tstart;
		fr = (tdelta != 0)? (1000.0 / tdelta): 1000;
	}
	LOG_DEBUG(("exiting main loop."));
	if (_running)
		throw_sdl(("SDL_WaitEvent"));
}

void IGame::deinit() {
	delete _server; _server = NULL;
	delete _client; _client = NULL;
	LOG_DEBUG(("shutting down, freeing surface"));
	_running = false;
	_window.free();
}

void IGame::notify(const PlayerState& state) {
	if (_client)
		_client->notify(state);
	if (_server) {
		mrt::Serializator s;
		World->serialize(s);

		Message message(UpdateWorld);
		message.data = s.getData();
		_server->broadcast(message);
	}
}

void IGame::onClient(Message &message) {
	const std::string an = "red-tank";
	LOG_DEBUG(("new client! spawning player:%s", an.c_str()));
	const int client_id = spawnPlayer("stateless-player", an, "network");
	LOG_DEBUG(("client #%d", client_id));

	LOG_DEBUG(("sending server status message..."));
	message.type = ServerStatus;
	message.set("map", Map->getName());
	message.set("version", getVersion());

	mrt::Serializator s;
	World->serialize(s);
	s.add(_players[client_id].obj->getID());

	message.data = s.getData();
	LOG_DEBUG(("world: %s", message.data.dump().c_str()));
}

void IGame::onMessage(const Connection &conn, const Message &message) {
	LOG_DEBUG(("incoming message %d", message.type));
	if (message.type == ServerStatus) {
		LOG_DEBUG(("loading map..."));
		Map->load(message.get("map"));
		
		mrt::Serializator s(&message.data);
		World->deserialize(s);
		int my_id;
		s.get(my_id);
		LOG_DEBUG(("my_id = %d", my_id));
		_players.clear();
		_my_index = 0;
		const Object * obj = World->getObjectByID(my_id);
		if (obj == NULL) 
			throw_ex(("invalid object id returned from server. (%d)", my_id));
		Object *player = const_cast<Object *>(obj); //fixme
		assert(player != NULL);
		_players.push_back(player);
		LOG_DEBUG(("players = %d", _players.size()));
	} else if (message.type == UpdateWorld) {
		mrt::Serializator s(&message.data);
		World->deserialize(s);
	}
}

