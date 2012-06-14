#include "menu.h"

#include <stdio.h>
#include <tcod/libtcod.h>

#include "orders.h"
#include "robot.h"
#include "task.h"
#include "map.h"
#include "building.h"

menu_state state;

void menu_init()
{
	state = MENU_NONE;
}

menu_state menu_get_state()
{
	return state;
}

void menu_set_state(menu_state new_state)
{
	state = new_state;
}

void menu_draw()
{
	// draw menu
	int linenum = 1;
	TCOD_console_print_frame(NULL, 0, 0, 30, 50, false, TCOD_BKGND_NONE, NULL);
	if (state == MENU_NONE)
	{
        TCOD_console_set_alignment(NULL, TCOD_LEFT);
		TCOD_console_print(NULL, 1, linenum++, "Menu");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		TCOD_console_print(NULL, 1, linenum++, "b - build");
		TCOD_console_print(NULL, 1, linenum++, "s - search");
		if (building_model_exists(BUILDING_LABORATORY))
			TCOD_console_print(NULL, 1, linenum++, "r - research");
		TCOD_console_print(NULL, 1, linenum++, "q - quit");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		TCOD_console_print(NULL, 1, linenum++, "The genius is...");
		if (robot_genius()->curr_task != NULL)
			TCOD_console_print(NULL, 3, linenum++, robot_genius()->curr_task->desc);
		else
			TCOD_console_print(NULL, 3, linenum++, "doing nothing");

		// print orders
		linenum++;
		TCOD_console_print(NULL, 1, linenum++, "Orders");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		order *o;
		for (o = orders_list(); o != NULL; o = o->next)
			TCOD_console_print(NULL, 1, linenum++, o->task->desc);

		// print storage
		linenum++;
		TCOD_console_print(NULL, 1, linenum++, "Storage");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		int i;
		for (i = 0; i < ITEM_COUNT; ++i)
			if (i == ITEM_SCRAP && storage_get_count(i) > 0)
				TCOD_console_print(NULL, 1, linenum++, "Screws: %d", storage_get_count(i));
	} else if (state == MENU_SEARCH) {
		TCOD_console_print(NULL, 1, linenum++, "Search Menu");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		TCOD_console_print(NULL, 1, linenum++, "s - scrap");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		TCOD_console_print(NULL, 1, linenum++, "esc - back");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
	} else if (state == MENU_BUILD) {
		TCOD_console_print(NULL, 1, linenum++, "Build Menu");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		TCOD_console_print(NULL, 1, linenum++, "l - laboratory");
		TCOD_console_print(NULL, 1, linenum++, "w - workshop");
		TCOD_console_print(NULL, 1, linenum++, "s - storage");
		TCOD_console_print(NULL, 1, linenum++, "q - quarry");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		if (research_is_completed(RESEARCH_ROBOT_GATHERER) && building_model_exists(BUILDING_WORKSHOP)) {
			TCOD_console_print(NULL, 1, linenum++, "g - gatherer robot");
			TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		}
		TCOD_console_print(NULL, 1, linenum++, "esc - back");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
	} else if (state == MENU_MOVEBUILDING) {
		TCOD_console_print(NULL, 1, linenum++, "Building Placement Menu");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		TCOD_console_print(NULL, 1, linenum++, "up - move building");
		TCOD_console_print(NULL, 1, linenum++, "down - move building");
		TCOD_console_print(NULL, 1, linenum++, "left - move building");
		TCOD_console_print(NULL, 1, linenum++, "right - move building");
		TCOD_console_print(NULL, 1, linenum++, "enter - place building");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		TCOD_console_print(NULL, 1, linenum++, "esc - back");
	} else if (state == MENU_RESEARCH) {
		TCOD_console_print(NULL, 1, linenum++, "Research Menu");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		TCOD_console_print(NULL, 1, linenum++, "g - gatherer");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		TCOD_console_print(NULL, 1, linenum++, "esc - back");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
	}
}
