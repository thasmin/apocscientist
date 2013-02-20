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

#include "point.h"
#include "robot.h"
#include "task.h"
#include "map.h"
#include "menu.h"
#include "building.h"
#include "orders.h"
#include "job.h"
#include "lualib.h"

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

int main(int argc, char* argv[])
{
	// initialize lua first so it's available to other inits
	L = luaL_newstate();
	luaL_openlibs(L);
	lualib_init(L);

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

	// clear lua stack of the return value of the create method, the item at index 0, and the tasks list
	lua_pop(L, 3);

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

	lua_close(L);

	return 0;
}
