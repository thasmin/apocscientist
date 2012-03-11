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

#define CHAR_BUCKET 	207
#define CHAR_WELL 	9
#define CHAR_GUY 	2
#define CHAR_SCREW 	21
#define CHAR_GATHERER	239

int symbols[ITEM_COUNT];
building *temp_building;

void setup_map()
{
	symbols[ITEM_BUCKET] = CHAR_BUCKET;
	symbols[ITEM_WELL] = CHAR_WELL;
	symbols[ITEM_SCREW] = CHAR_SCREW;

	// house is 50,15 to 70,35
	// make 5 screws outside house
	for (int i = 0; i < 2; ++i) {
		int x, y;
		do {
			x = rand() % 49 + 31;
			y = rand() % 49 +  1;
		} while (x < 50 || x > 70 || y < 15 || y < 35);
		map_create_item(x, y, ITEM_SCREW);
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
			point_moveto(&robot_genius()->p, &data->bucket, d->model.speed * frameduration);
			if (point_equals(&robot_genius()->p, &data->bucket))
				t->stage++;
			return 1;
		case 1:
			robot_pickup(robot_genius());
			t->stage++;
			return 1;
		case 2:
			point_moveto(&robot_genius()->p, &data->well, d->model.speed * frameduration);
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

	robot_genius()->p.x = 46;
	robot_genius()->p.y = 22;
	robot_create(ROBOT_GATHERER, 47, 22);
}

int main(int argc, char* argv[])
{
	// initialize data structures
	research_init();
	map_init();
	robots_init();
	buildings_init();
	orders_init();
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

	TCOD_key_t key = {TCODK_NONE,0};

	TCOD_console_init_root(MAP_COLS, MAP_ROWS, "Square Civilization", false, TCOD_RENDERER_GLSL);
	TCOD_sys_set_fps(30);
	TCOD_console_set_default_background(NULL, TCOD_black);
	TCOD_console_set_default_foreground(NULL, TCOD_light_gray);

	do {
		TCOD_console_clear(NULL);

		// give orders to idle robots
		robot *d;
		robot_reset_idle();
		for (d = robot_next_idle(); d != NULL; d = robot_next_idle()) {
			d->curr_task = order_next(d);
			if (d->curr_task == NULL) {
				d->curr_task = task_clone(d->model.idle_task);
			}
		}

		robots_act(TCOD_sys_get_last_frame_length());

		// draw map
		for (int i = 0; i < MAP_COLS; ++i)
			for (int j = 0; j < MAP_ROWS; ++j)
				if (map_item_at(i,j) != ITEM_NONE)
					TCOD_console_set_char(NULL, i, j, symbols[map_item_at(i, j)]);

		// draw robots
		for (d = robot_genius(); d != NULL; d = d->next)
			TCOD_console_set_char(NULL, d->p.x, d->p.y, d->model.mapchar);

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
			} else if (menu_get_state() == MENU_SEARCH) {
				if (key.vk == TCODK_ESCAPE)
					menu_set_state(MENU_NONE);
				else if (key.c == 's' || key.c == 'S') {
					order_add(task_search_create(ITEM_SCREW));
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
				} else if (key.c == 'g' || key.c == 'G') {
					menu_set_state(MENU_NONE);
					if (research_is_completed(RESEARCH_ROBOT_GATHERER) &&
						building_model_exists(BUILDING_WORKSHOP) &&
						storage_get_count(ITEM_SCREW) >= 2) {
						order_add(
							task_build_create(BUILDABLE_ROBOT_GATHERER)
						);
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
			}
		}
	} while (!TCOD_console_is_window_closed());

	map_destroy();

	return 0;
}
