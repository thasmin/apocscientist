#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <time.h>
#include <tcod/libtcod.h>

#include "point.h"
#include "dwarf.h"
#include "task.h"
#include "map.h"
#include "menu.h"
#include "building.h"
#include "orders.h"

#define CHAR_BUCKET 	207
#define CHAR_WELL 	9
#define CHAR_GUY 	2
#define CHAR_SCREW 	21

int symbols[ITEM_COUNT];
building *temp_building;

// guy starts at 40,10 and has 12 speed
dwarf guy = { { 40, 10 }, 12 };

void setup_map()
{
	symbols[ITEM_BUCKET] = CHAR_BUCKET;
	symbols[ITEM_WELL] = CHAR_WELL;
	symbols[ITEM_SCREW] = CHAR_SCREW;

	// frame is 0, 0 to 30, 50

	map_create_item(45, 20, ITEM_BUCKET);
	map_create_item(50, 40, ITEM_BUCKET);
	map_create_item(70, 20, ITEM_BUCKET);

	map_create_item(60, 25, ITEM_WELL);

	// house is 50,15 to 70,35
	// make 5 screws outside house
	for (int i = 0; i < 5; ++i) {
		int x, y;
		do {
			x = rand() % 49 + 31;
			y = rand() % 49 +  1;
		} while (x < 50 || x > 70 || y < 15 || y < 35);
		map_create_item(x, y, ITEM_SCREW);
	}
}

void destroy_map()
{
	int i;
	for (i = 0; i < ITEM_COUNT; ++i)
	{
		point_node *node = map_item_list(i);
		while (node != NULL) {
			point_node *next = node->next;
			free(node);
			node = next;
		}
	}
}

typedef struct {
	point bucket;
	point well;
} task_pour_bucket_data;

int task_pour_bucket_act(dwarf *d, float frameduration)
{
	task *t = d->curr_task;
	task_pour_bucket_data* data = (task_pour_bucket_data*)t->localdata;

	// stage 0 is move to bucket
	// stage 1 is picket bucket
	// stage 2 is move to well
	// stage 3 is pour bucket
	switch (t->stage) {
		case 0:
			point_moveto(&guy.p, &data->bucket, d->speed * frameduration);
			if (point_equals(&guy.p, &data->bucket))
				t->stage++;
			return 1;
		case 1:
			dwarf_pickup(&guy);
			t->stage++;
			return 1;
		case 2:
			point_moveto(&guy.p, &data->well, d->speed * frameduration);
			if (point_equals(&guy.p, &data->well))
				t->stage++;
			return 1;
		case 3:
			dwarf_consume(&guy);
			return 0;
	}
	return 0;
}

void setup_orders()
{
	// set up guy to pour buckets on repeat
	point* bucket = map_find_closest(&guy.p, ITEM_BUCKET);
	if (bucket == NULL)
		return;
	point* well = map_find_closest(&guy.p, ITEM_WELL);
	if (well == NULL)
		return;

	task *t = malloc(sizeof(task));
	t->desc = "Pour a bucket into a well";
	t->assign = NULL;
	t->act = task_pour_bucket_act;
	t->stage = 0;
	t->localdata = malloc(sizeof(task_pour_bucket_data));
	task_pour_bucket_data *data = (task_pour_bucket_data*)t->localdata;
	memcpy(&data->bucket, bucket, sizeof(point));
	memcpy(&data->well, well, sizeof(point));

	order_add(t);
}

int main(int argc, char* argv[])
{
	int i, j;

	dwarves_init();
	buildings_init();
	orders_init();
	temp_building = NULL;

	//srand(time(NULL));
	setup_map();
	setup_orders();
	dwarf_add(&guy);

	TCOD_key_t key = {TCODK_NONE,0};

	TCOD_console_init_root(MAP_COLS, MAP_ROWS, "Square Civilization", false);
	TCOD_sys_set_fps(30);
	TCOD_console_set_background_color(NULL, TCOD_black);
	TCOD_console_set_foreground_color(NULL, TCOD_light_gray);

	do {
		TCOD_console_clear(NULL);

		// give orders to idle dwarves
		dwarf *d;
		dwarf_reset_idle();
		for (d = dwarf_next_idle(); d != NULL; d = dwarf_next_idle())
			d->curr_task = order_next(d);

		dwarves_act(TCOD_sys_get_last_frame_length());

		for (i = 0; i < MAP_COLS; ++i)
			for (j = 0; j < MAP_ROWS; ++j)
				if (map_item_at(i,j) != ITEM_NONE)
					TCOD_console_set_char(NULL, i, j, symbols[map_item_at(i, j)]);
		TCOD_console_set_char(NULL, (int)guy.p.x, (int)guy.p.y, CHAR_GUY);

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
				}
			} else if (menu_get_state() == MENU_MOVEBUILDING) {
				if (key.vk == TCODK_ESCAPE) {
					free(temp_building);
					temp_building = NULL;
					menu_set_state(MENU_NONE);
				} else if (key.vk == TCODK_ENTER) {
					order_add(task_build_create(
						temp_building->model->model,
						temp_building->x,
						temp_building->y));
					free(temp_building);
					temp_building = NULL;
					menu_set_state(MENU_NONE);
				} else if (key.vk == TCODK_UP)
					temp_building->y -= 1;
				else if (key.vk == TCODK_DOWN)
					temp_building->y += 1;
				else if (key.vk == TCODK_LEFT)
					temp_building->x -= 1;
				else if (key.vk == TCODK_RIGHT)
					temp_building->x += 1;
			}
		}
	} while (!TCOD_console_is_window_closed());

	destroy_map();

	return 0;
}
