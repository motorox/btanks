import sys

Import('env')
Import('version')
Import('revision')
Import('debug')
Import('sigc_lib')
Import('al_lib')

env = env.Copy()
venv = env.Copy()
bt_env = env.Copy()

venv.Append(CPPDEFINES=['VERSION="\\"' + version + '\\""'])
venv.Append(CPPDEFINES=['REVISION=%d' % revision])

venv.Append(CPPDEFINES=['BTANKSAPI=DLLEXPORT']);
env.Append(CPPDEFINES=['BTANKSAPI=DLLEXPORT']);

vobj = venv.SharedObject('src/version.cpp')
bt_sources = 	[
#networking
	'net/message.cpp', 'net/server.cpp', 'net/client.cpp', 'net/connection.cpp', 'net/monitor.cpp', 'net/scanner.cpp', 
#menu
	'menu/checkbox.cpp', 'menu/shop.cpp', 'menu/image_view.cpp', 'menu/shop_item.cpp', 'menu/popup_menu.cpp', 
	'menu/start_server_menu.cpp', 'menu/join_server_menu.cpp', 'menu/campaign_menu.cpp', 'menu/chat.cpp',
	'menu/menu.cpp', 'menu/box.cpp', 'menu/upper_box.cpp', 'menu/base_menu.cpp', 'menu/container.cpp', 'menu/menuitem.cpp',
	'menu/map_picker.cpp', 'menu/scroll_list.cpp', 'menu/control.cpp', 'menu/map_details.cpp', 'menu/player_picker.cpp', 
	'menu/chooser.cpp', 'menu/label.cpp', 'menu/button.cpp', 'menu/menu_config.cpp', 'menu/map_desc.cpp', 
	'menu/prompt.cpp', 'menu/text_control.cpp', 'menu/host_list.cpp', 'menu/options_menu.cpp', 'menu/control_picker.cpp',
	'menu/slider.cpp', 'menu/redefine_keys.cpp', 'menu/gamepad_setup.cpp', 'menu/tooltip.cpp', 'menu/player_name_control.cpp', 
	'menu/host_item.cpp', 'menu/number_control.cpp', 
#end of menu

	'src/finder.cpp', 'src/zbox.cpp', 'src/campaign.cpp', 'src/netstats.cpp', 
	
	'src/alarm.cpp', 'src/base_object.cpp', 'src/notifying_xml_parser.cpp', 'src/xml_parser.cpp', 
	'src/special_zone.cpp', 'src/game_monitor.cpp', 
	'src/player_manager.cpp', 'src/object_grid.cpp', 'src/variants.cpp', 

	'ai/base.cpp', 'ai/buratino.cpp', 'ai/herd.cpp', 'ai/traits.cpp', 'ai/waypoints.cpp', 'ai/rush.cpp', 'ai/old_school.cpp', 'ai/trooper.cpp', 
		
	'src/player_state.cpp', 'controls/joy_bindings.cpp', 
	'controls/joyplayer.cpp', 'controls/keyplayer.cpp', 'controls/external_control.cpp', 'controls/mouse_control.cpp', 

	'src/object.cpp', 'src/animation_model.cpp', 
	'src/resource_manager.cpp', 'src/world.cpp',
	
	'tmx/generator_object.cpp', 'tmx/tileset.cpp', 'tmx/generator.cpp', 'tmx/tileset_list.cpp',  
	'tmx/map.cpp', 'tmx/layer.cpp',
	
	'src/var.cpp', 'src/config.cpp', 'src/registrar.cpp', 'src/profiler.cpp', 
	
	'src/player_slot.cpp', 'src/hud.cpp', 'src/console.cpp',
	'src/i18n.cpp', 'src/game.cpp', 'src/window.cpp', 
	'src/credits.cpp', 'src/cheater.cpp', 
	'src/vehicle_traits.cpp', 'src/nickname.cpp',
	
	'sound/al_ex.cpp', 'sound/mixer.cpp', 'sound/ogg_ex.cpp', 'sound/ogg_stream.cpp', 'sound/sample.cpp',

	vobj
	]

vorbis = 'vorbisfile'
if debug and sys.platform == "win32": 
	vorbis = 'vorbisfile_d'

bt_libs = ['mrt', 'zzip', 'sdlx',  sigc_lib, 'SDL', vorbis, al_lib]

if sys.platform != 'win32' and env['enable_lua']:
	got_lua = False
	for lua in ['lua5.1', 'lua5.0', 'lua']: 
		try: 
			env.ParseConfig("pkg-config --cflags --libs %s 2>/dev/null" %lua);
			got_lua = True
			env.Append(CPPDEFINES=['ENABLE_LUA'])
			bt_libs.append(lua)
			break
		except: 
			continue
	if not got_lua: 
		raise Exception("Cannot find lua5.1/lua5.0 or lua pkg-config information")

if env['enable_lua']: 
	env.Append(CPPDEFINES=['ENABLE_LUA'])
	bt_sources.append('luaxx/state.cpp')
	bt_sources.append('luaxx/lua_hooks.cpp')
	if sys.platform == "win32":
		bt_libs.append('lua')

if sys.platform == "win32":
	bt_libs[0:0] = ['SDLmain']
	bt_libs.append('Ws2_32')
	bt_libs.append('user32')
	#bt_libs.append('gdi32')
#	bt_libs.append('opengl32')
#else: 
#	bt_libs.append('GL')


bt = env.SharedLibrary('bt', bt_sources, LIBS=bt_libs, RPATH=['.'])
Install('#', bt[0])

bt_main_sources = ['src/main.cpp']
bt_main_libs =  ['mrt', 'bt', 'SDL', 'sdlx']

if sys.platform == "win32":
	bt_main_sources.append('sdlx/SDL_win32_main.c')
	bt_rc = env.RES('src/bt.rc')
	bt_main_sources.append(bt_rc)
	bt_main_libs.append('user32')


Import('lib_dir')
if sys.platform != 'win32':
	bt_env.Append(LINKFLAGS=' -Wl,-rpath,'+ lib_dir + ' -Wl,-rpath-link,build/' + env['mode'] + '/mrt')
	bt_env.Append(LINKFLAGS=' -Wl,-rpath,'+ lib_dir + ' -Wl,-rpath-link,build/' + env['mode'] + '/sdlx')
	bt_env.Append(LINKFLAGS=' -Wl,-rpath,'+ lib_dir + ' -Wl,-rpath-link,build/' + env['mode'] + '/zzip')

bt_main = bt_env.Program('bt', bt_main_sources, LIBS=bt_main_libs, RPATH=[lib_dir])
Install('#', bt_main)
