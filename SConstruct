env = Environment()

opts = Options(['options.cache'])
opts.Add('CC', 'C compiler')
opts.Add('CXX', 'C++ compiler')
opts.Add('CFLAGS', 'General options that are passed to the C compiler', '')
opts.Add('CXXFLAGS', 'General options that are passed to the C++ compiler', '')

opts.Update(env)

opts.Save('options.cache', env.Copy())

Help(opts.GenerateHelpText(env))


conf_env = env.Copy()
conf = Configure(conf_env)

sigc_cpppath = ['/usr/lib/sigc++-2.0/include', '/usr/include/sigc++-2.0', '/usr/local/include/sigc++-2.0', '/usr/local/lib/sigc++-2.0/include']
sigc_lib = 'sigc-2.0'

conf_env.Prepend(CPPPATH=sigc_cpppath)

if not conf.CheckLibWithHeader(sigc_lib, 'sigc++/sigc++.h', 'c++', "SigC::Signal1<int,int> sig;", False):
	Exit(1)

if not conf.CheckLibWithHeader('SDL', 'SDL/SDL.h', 'c++', "SDL_Init(0);", False):
	Exit(1)

if not conf.CheckLibWithHeader('SDL_image', 'SDL/SDL_image.h', 'c++', "IMG_Load(0);", False):
	Exit(1)

if not conf.CheckLibWithHeader('SDL_ttf', 'SDL/SDL_ttf.h', 'c++', "TTF_Init();", False):
	Exit(1)

if not conf.CheckLibWithHeader('SDL_gfx', 'SDL/SDL_framerate.h', 'c++', "SDL_initFramerate(0);", False):
	Exit(1)

if not conf.CheckLibWithHeader('expat', 'expat.h', 'c', "XML_ParserCreate(NULL);", False):
	Exit(1)

if not conf.CheckLibWithHeader('z', 'zlib.h', 'c', "zlibVersion();", False):
	Exit(1)

conf.Finish()

env.Append(CPPFLAGS=' -Wall -pedantic -ggdb3 -Wno-long-long')
#env.Append(LINKFLAGS=' -pg ')
#env.Append(CPPFLAGS=' -O')

Export('env')
Export('sigc_cpppath')
Export('sigc_lib')

SConscript('mrt/SConscript')
SConscript('sdlx/SConscript')

env = env.Copy()
env.Append(LIBPATH=['mrt', 'sdlx', 'src'])
env.Prepend(CPPPATH=['.', 'src'])
env.Append(CPPPATH=sigc_cpppath)

xc = env.Program('bt', 
	['src/object.cpp', 'objects/bullet.cpp', 'objects/explosion.cpp', 

	'src/menu.cpp', 'src/menuitem.cpp', 'src/joyplayer.cpp', 'src/keyplayer.cpp', 
	 'src/player.cpp', 'src/aiplayer.cpp',
	'src/animated_object.cpp', 'src/animation_model.cpp', 
	'src/resource_manager.cpp', 'src/world.cpp',
	'tmx/map.cpp', 'tmx/layer.cpp', 
	'sdl_collide/SDL_collide.c', 
	'src/main.cpp', 'src/game.cpp', 
	], 
LIBS=['sdlx', 'mrt', sigc_lib, 'SDL_gfx', 'SDL_ttf', 'SDL_image', 'SDL', 'expat', 'z'], RPATH=['.'])
