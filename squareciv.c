#include <stdio.h>
#include <string.h>
#ifdef MACOSX
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
#include <math.h>
#include <time.h>
#include <getopt.h>

#include <tcod/libtcod.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "point.h"
#include "robot.h"
#include "task.h"
#include "map.h"
#include "menu.h"
#include "building.h"
#include "orders.h"
#include "job.h"

#define CHAR_BUCKET 	207
#define CHAR_WELL		9
#define CHAR_GUY		2
#define CHAR_SCRAP		21
#define CHAR_ROCK		8
#define CHAR_GATHERER	239

int symbols[ITEM_COUNT];
building *temp_building;

void setup_map()
{
	symbols[ITEM_BUCKET] = CHAR_BUCKET;
	symbols[ITEM_WELL] = CHAR_WELL;
	symbols[ITEM_SCRAP] = CHAR_SCRAP;
	symbols[ITEM_ROCK] = CHAR_ROCK;

	// house is 50,15 to 70,35
	// make 5 screws outside house
	for (int i = 0; i < 2; ++i) {
		int x, y;
		do {
			x = rand() % 49 + 31;
			y = rand() % 49 +  1;
		} while (x < 50 || x > 70 || y < 15 || y < 35);
		map_create_item(x, y, ITEM_SCRAP);
	}
}

typedef struct {
	point bucket;
	point well;
} task_pour_bucket_data;

int task_pour_bucket_act(robot *d, float frameduration)
{
	task *t = d->curr_task;
	task_pour_bucket_data* data = (task_pour_bucket_data*)t->localdata;

	// stage 0 is move to bucket
	// stage 1 is picket bucket
	// stage 2 is move to well
	// stage 3 is pour bucket
	switch (t->stage) {
		case 0:
			point_moveto(&robot_genius()->p, &data->bucket, d->speed * frameduration);
			if (point_equals(&robot_genius()->p, &data->bucket))
				t->stage++;
			return 1;
		case 1:
			robot_pickup(robot_genius());
			t->stage++;
			return 1;
		case 2:
			point_moveto(&robot_genius()->p, &data->well, d->speed * frameduration);
			if (point_equals(&robot_genius()->p, &data->well))
				t->stage++;
			return 1;
		case 3:
			robot_consume(robot_genius());
			return 0;
	}
	return 0;
}

void setup_scenario_zero()
{
	// frame is 0, 0 to 30, 50
	map_create_item(45, 20, ITEM_BUCKET);
	map_create_item(50, 40, ITEM_BUCKET);
	map_create_item(70, 20, ITEM_BUCKET);
	map_create_item(60, 25, ITEM_WELL);

	// set up genius to pour buckets on repeat
	point* bucket = map_find_closest(&robot_genius()->p, ITEM_BUCKET);
	if (bucket == NULL)
		return;
	point* well = map_find_closest(&robot_genius()->p, ITEM_WELL);
	if (well == NULL)
		return;

	task *t = malloc(sizeof(task));
	t->desc = "Pour a bucket into a well";
	t->act = task_pour_bucket_act;
	t->stage = 0;
	t->localdata = malloc(sizeof(task_pour_bucket_data));
	task_pour_bucket_data *data = (task_pour_bucket_data*)t->localdata;
	memcpy(&data->bucket, bucket, sizeof(point));
	memcpy(&data->well, well, sizeof(point));

	order_add(t);
}

void setup_scenario_one()
{
	point lab = { 50, 20 };
	building_add(BUILDING_LABORATORY, &lab);
	point workshop = { 44, 20 };
	building_add(BUILDING_WORKSHOP, &workshop);
	point storage = { 50, 26 };
	building_add(BUILDING_STORAGE, &storage);
	point quarry = { 64, 30 };
	building_add(BUILDING_QUARRY, &quarry);

	robot_genius()->p.x = 46;
	robot_genius()->p.y = 22;
	robot *gatherer = robot_create("gatherer", 47, 22);
	robot_set_job(gatherer, JOB_GATHERER);
	robot *miner = robot_create("gatherer", 47, 32);
	robot_set_job(miner, JOB_MINER);
}

const char *LUA_MT_ROBOT = "robot";
const char *LUA_MT_POINT = "point";
const char *LUA_MT_BUILDING = "building";
const char *LUA_MT_BUILDING_MODEL = "building_model";

building* lh_push_building(lua_State *L, building *src)
{
	building *b = (building *)lua_newuserdata(L, sizeof(building));
	luaL_getmetatable(L, LUA_MT_BUILDING);
	lua_setmetatable(L, -2);
	memcpy(b, src, sizeof(building));
	return b;
}

building_model* lh_push_building_model(lua_State *L, building_model *src)
{
	building_model *b = (building_model *)lua_newuserdata(L, sizeof(building_model));
	luaL_getmetatable(L, LUA_MT_BUILDING_MODEL);
	lua_setmetatable(L, -2);
	memcpy(b, src, sizeof(building_model));
	return b;
}

robot* lh_push_robot(lua_State *L, robot *src)
{
	robot *r = (robot *)lua_newuserdata(L, sizeof(robot));
	luaL_getmetatable(L, LUA_MT_ROBOT);
	lua_setmetatable(L, -2);
	memcpy(r, src, sizeof(robot));
	return r;
}

point* lh_push_point(lua_State *L, point *src)
{
	point *p = (point *)lua_newuserdata(L, sizeof(point));
	luaL_getmetatable(L, LUA_MT_POINT);
	lua_setmetatable(L, -2);
	if (src != NULL)
		memcpy(p, src, sizeof(point));
	return p;
}

int l_point_new(lua_State *L)
{
	point *p = lh_push_point(L, NULL);
	p->x = luaL_checknumber(L, 1);
	p->y = luaL_checknumber(L, 2);
	return 1;
}

const struct luaL_reg point_lib[] = {
	{"new", l_point_new},
	{NULL, NULL},
};

int l_buildings_find_closest(lua_State *L)
{
	point *p = (point*) luaL_checkudata(L, 1, LUA_MT_POINT);
	building *b = building_find_closest(p, BUILDING_QUARRY);
	lh_push_building(L, b);
	return 1;
}

const struct luaL_reg buildings_lib[] = {
	{"find_closest", l_buildings_find_closest},
	{NULL, NULL},
};

int l_building_model__index(lua_State *L)
{
	building_model *m = (building_model *) luaL_checkudata(L, 1, LUA_MT_BUILDING_MODEL);
	const char* prop = lua_tostring(L, 2);
	if (strcmp(prop, "width") == 0) {
		lua_pushinteger(L, m->width);
		return 1;
	} else 	if (strcmp(prop, "height") == 0) {
		lua_pushinteger(L, m->height);
		return 1;
	}

	return 0;
}

const struct luaL_Reg l_building_model_reg[] = {
	{ "__index", l_building_model__index } ,
	{ NULL, NULL }
};

int l_building__index(lua_State *L)
{
	building *b = (building*) luaL_checkudata(L, 1, LUA_MT_BUILDING);
	const char* prop = lua_tostring(L, 2);
	if (strcmp(prop, "p") == 0) {
		lh_push_point(L, &b->p);
		return 1;
	} else 	if (strcmp(prop, "model") == 0) {
		lh_push_building_model(L, b->model);
		return 1;
	}

	return 0;
}

int l_building__tostring(lua_State *L)
{
	building *b = (building*) luaL_checkudata(L, 1, LUA_MT_BUILDING);
	lua_pushfstring(L, "building at %f, %f", b->p.x, b->p.y);
	return 1;
}

const struct luaL_Reg l_building_reg[] = {
	{ "__index", l_building__index } ,
	{ "__tostring", l_building__tostring } ,
	{ NULL, NULL }
};

int l_robot__index(lua_State *L)
{
	robot *r = (robot*) luaL_checkudata(L, 1, LUA_MT_ROBOT);
	const char* prop = lua_tostring(L, 2);
	if (strcmp(prop, "p") == 0) {
		lh_push_point(L, &r->p);
		return 1;
	}
	return 0;
}

int l_robot__tostring(lua_State *L)
{
	robot *r = (robot*) luaL_checkudata(L, 1, LUA_MT_ROBOT);
	lua_pushfstring(L, "robot at %f, %f", r->p.x, r->p.y);
	return 1;
}

const struct luaL_Reg l_robot_reg[] = {
	{ "__index", l_robot__index } ,
	{ "__tostring", l_robot__tostring } ,
	{ NULL, NULL }
};

int l_point__index(lua_State *L)
{
	point *p = (point*) luaL_checkudata(L, 1, LUA_MT_POINT);
	const char* prop = lua_tostring(L, 2);
	if (strcmp(prop, "x") == 0) {
		lua_pushnumber(L, p->x);
		return 1;
	} else if (strcmp(prop, "y") == 0) {
		lua_pushnumber(L, p->y);
		return 1;
	}
	return 0;
}

int l_point__tostring(lua_State *L)
{
	point *p = (point*) luaL_checkudata(L, 1, LUA_MT_POINT);
	lua_pushfstring(L, "point at %f, %f", p->x, p->y);
	return 1;
}

const struct luaL_Reg l_point_reg[] = {
	{ "__index", l_point__index },
	{ "__tostring", l_point__tostring },
	{ NULL, NULL },
};

int main(int argc, char* argv[])
{
	// initialize data structures
	research_init();
	map_init();
	robots_init();
	buildings_init();
	orders_init();
	job_init();
	temp_building = NULL;

	// global options
	int debug = 0;
	int scenario = 0;
	struct option options[] = {
		{ "debug", no_argument, 0, 'd' },
		{ "scenario", required_argument, 0, 's' },
		{ 0, 0, 0, 0},
	};
	int c, option_index = 0;
	while ((c = getopt_long(argc, argv, "ds:", options, &option_index)) != -1) {
		switch (c) {
			case 0:
				break;
			case 'd':
				debug = 1;
				break;
			case 's':
				scenario = atoi(optarg);
				break;
		}
	}

	//srand(time(NULL));
	setup_map();
	if (debug && scenario == 0)
		setup_scenario_zero();
	else if (debug && scenario == 1)
		setup_scenario_one();

	/*******************************/
	// load lua scripts
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	int err = luaL_loadfile(L, "tasks/mine.lua");
	if (err != 0) {
		printf("loadfile error: %d\n", err);
		printf("LUA_ERRSYNTAX: %d\n", LUA_ERRSYNTAX);
		printf("LUA_ERRMEM: %d\n", LUA_ERRMEM);
		printf("LUA_ERRFILE: %d\n", LUA_ERRFILE);
		exit(1);
	}
	err = lua_pcall(L, 0, LUA_MULTRET, 0);
	if (err != 0) {
		printf("pcall error: %d\n", err);
		printf("LUA_ERRRUN: %d\n", LUA_ERRRUN);
		printf("LUA_ERRMEM: %d\n", LUA_ERRMEM);
		printf("LUA_ERRERR: %d\n", LUA_ERRERR);
		exit(1);
	}

	// create robot object functions
	luaL_newmetatable(L, LUA_MT_BUILDING);
	luaL_register(L, NULL, l_building_reg);
	luaL_newmetatable(L, LUA_MT_BUILDING_MODEL);
	luaL_register(L, NULL, l_building_model_reg);
	luaL_newmetatable(L, LUA_MT_ROBOT);
	luaL_register(L, NULL, l_robot_reg);
	luaL_newmetatable(L, LUA_MT_POINT);
	luaL_register(L, NULL, l_point_reg);
	lua_pop(L, 4);

	// create global functions
	luaL_openlib(L, "buildings", buildings_lib, 0);
	luaL_openlib(L, "point", point_lib, 0);
	lua_pop(L, 2);

	// load tasks, get the item at index 0, then get the create method
	lua_getglobal(L, "tasks");
	lua_pushinteger(L, 1);
	lua_gettable(L, -2);
	lua_pushstring(L, "create");
	lua_gettable(L, -2);

	// push a robot onto the stack and call the function
	lh_push_robot(L, robot_genius());
	err = lua_pcall(L, 1, 1, 0);
	if (err != 0)
		printf("error: %s\n", lua_tostring(L, -1));
	else
		printf("result: %g\n", lua_tonumber(L, -1));
//printf("top of lua stack is a %s\n", luaL_typename(L, -1));

	// clear lua stack of the return value of the create method, the item at index 0, and the tasks list
	lua_pop(L, 3);

	lua_close(L);
	exit(0);

	/*******************************/

	TCOD_key_t key = {TCODK_NONE,0};

	TCOD_console_init_root(MAP_COLS, MAP_ROWS, "Square Civilization", false, TCOD_RENDERER_GLSL);
	TCOD_sys_set_fps(30);
	TCOD_console_set_default_background(NULL, TCOD_black);
	TCOD_console_set_default_foreground(NULL, TCOD_light_gray);

	int game_paused = 0;
	robot *chosen_robot = NULL;

	do {
		TCOD_console_clear(NULL);

		if (!game_paused) {
			// give orders to idle robots
			robot *d;
			robot_reset_idle();
			while ((d = robot_next_idle()) != NULL) {
				d->curr_task = order_next(d);
				if (d->curr_task == NULL && d->job->idle_task != NULL)
					d->curr_task = d->job->idle_task(d);
			}

			robots_act(TCOD_sys_get_last_frame_length());
		}

		// draw map
		for (int i = 0; i < MAP_COLS; ++i)
			for (int j = 0; j < MAP_ROWS; ++j)
				if (map_item_at(i,j) != ITEM_NONE)
					TCOD_console_set_char(NULL, i, j, symbols[map_item_at(i, j)]);

		// draw robots
		robot *d;
		for (d = robot_genius(); d != NULL; d = d->next) {
			if (menu_get_state() == MENU_CHOOSE_ROBOT && chosen_robot == d)
				TCOD_console_print_frame(NULL, d->p.x - 1, d->p.y - 1, 3, 3, false, TCOD_BKGND_NONE, NULL);
			TCOD_console_set_char(NULL, d->p.x, d->p.y, d->job->mapchar);
		}

		buildings_draw();
		if (temp_building != NULL)
			building_draw(temp_building);
		menu_draw();

		TCOD_console_flush();

		/* did the user hit a key ? */
		key = TCOD_console_check_for_keypress(TCOD_KEY_PRESSED);
		if (key.vk != TCODK_NONE) {
			if (menu_get_state() == MENU_NONE) {
				if (key.c == 'q' || key.c == 'Q')
					break;
				else if (key.c == 's' || key.c == 'S')
					menu_set_state(MENU_SEARCH);
				else if (key.c == 'b' || key.c == 'B')
					menu_set_state(MENU_BUILD);
				else if (key.c == 'r' || key.c == 'R')
					menu_set_state(MENU_RESEARCH);
				else if (key.c == 'v' || key.c == 'V') {
					menu_set_state(MENU_CHOOSE_ROBOT);
					game_paused = 1;
					chosen_robot = robot_genius();
				}
			} else if (menu_get_state() == MENU_SEARCH) {
				if (key.vk == TCODK_ESCAPE)
					menu_set_state(MENU_NONE);
				else if (key.c == 's' || key.c == 'S') {
					order_add(task_search_create(ITEM_SCRAP));
					menu_set_state(MENU_NONE);
				}
			} else if (menu_get_state() == MENU_BUILD) {
				if (key.vk == TCODK_ESCAPE)
					menu_set_state(MENU_NONE);
				else if (key.c == 'l' || key.c == 'L') {
					menu_set_state(MENU_MOVEBUILDING);
					point p = { 52, 22 };
					temp_building = building_create(BUILDING_LABORATORY, &p);
				} else if (key.c == 'w' || key.c == 'W') {
					menu_set_state(MENU_MOVEBUILDING);
					point p = { 52, 22 };
					temp_building = building_create(BUILDING_WORKSHOP, &p);
				} else if (key.c == 's' || key.c == 'S') {
					menu_set_state(MENU_MOVEBUILDING);
					point p = { 52, 22 };
					temp_building = building_create(BUILDING_STORAGE, &p);
				} else if (key.c == 'q' || key.c == 'Q') {
					menu_set_state(MENU_MOVEBUILDING);
					point p = { 52, 22 };
					temp_building = building_create(BUILDING_QUARRY, &p);
				} else if (key.c == 'g' || key.c == 'G') {
					menu_set_state(MENU_NONE);
					if (research_is_completed(RESEARCH_ROBOT_GATHERER) &&
						building_model_exists(BUILDING_WORKSHOP) &&
						storage_get_count(ITEM_SCRAP) >= 2) {
						order_add(task_build_create(BUILDABLE_ROBOT_GATHERER));
					} else {
						menu_set_message("need research, workshop, 2 scrap");
					}
				}
			} else if (menu_get_state() == MENU_MOVEBUILDING) {
				if (key.vk == TCODK_ESCAPE) {
					free(temp_building);
					temp_building = NULL;
					menu_set_state(MENU_NONE);
				} else if (key.vk == TCODK_ENTER) {
					order_add(task_construct_create(
						temp_building->model->model,
						temp_building->p.x,
						temp_building->p.y));
					free(temp_building);
					temp_building = NULL;
					menu_set_state(MENU_NONE);
				} else if (key.vk == TCODK_UP)
					temp_building->p.y -= 1;
				else if (key.vk == TCODK_DOWN)
					temp_building->p.y += 1;
				else if (key.vk == TCODK_LEFT)
					temp_building->p.x -= 1;
				else if (key.vk == TCODK_RIGHT)
					temp_building->p.x += 1;
			} else if (menu_get_state() == MENU_RESEARCH) {
				if (key.vk == TCODK_ESCAPE)
					menu_set_state(MENU_NONE);
				else if (key.c == 'g' || key.c == 'G') {
					order_add(task_research_create(RESEARCH_ROBOT_GATHERER));
					menu_set_state(MENU_NONE);
				}
			} else if (menu_get_state() == MENU_CHOOSE_ROBOT) {
				if (key.vk == 'j' || key.vk == 'J')
					menu_set_state(MENU_ASSIGN_JOB);
				else if (key.vk == TCODK_ESCAPE) {
					game_paused = 0;
					chosen_robot = NULL;
					menu_set_state(MENU_NONE);
				}
			} else if (menu_get_state() == MENU_ASSIGN_JOB) {
				if (key.vk == 'm' || key.vk == 'm') {
					robot_set_job(chosen_robot, JOB_MINER);
					game_paused = 0;
					chosen_robot = NULL;
					menu_set_state(MENU_ASSIGN_JOB);
				} else if (key.vk == TCODK_ESCAPE) {
					menu_set_state(MENU_CHOOSE_ROBOT);
				}
			}

		}
	} while (!TCOD_console_is_window_closed());

	map_destroy();

	return 0;
}
