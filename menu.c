#include "menu.h"

#include <stdio.h>
#include <tcod/libtcod.h>

#include "orders.h"
#include "dwarf.h"
#include "task.h"
#include "map.h"
#include "building.h"

extern dwarf guy;

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
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "Menu");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "b - build");
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "s - search");
		if (building_model_exists(BUILDING_LABORATORY))
			TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "s - research");
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "q - quit");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "The dwarf is...");
		if (guy.curr_task != NULL)
			TCOD_console_print_left(NULL, 3, linenum++, TCOD_BKGND_NONE, guy.curr_task->desc);
		else
			TCOD_console_print_left(NULL, 3, linenum++, TCOD_BKGND_NONE, "doing nothing");

		// print orders
		linenum++;
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "Orders");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		order *o;
		for (o = orders_list(); o != NULL; o = o->next)
			TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, o->task->desc);

		// print storage
		linenum++;
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "Storage");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		int i;
		for (i = 0; i < ITEM_COUNT; ++i)
			if (i == ITEM_SCREW && storage_get_count(i) > 0)
				TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "Screws: %d", storage_get_count(i));
	} else if (state == MENU_SEARCH) {
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "Search Menu");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "s - screws");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "esc - back");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
	} else if (state == MENU_BUILD) {
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "Build Menu");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "l - laboratory");
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "w - workshop");
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "s - storage");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		if (research_is_completed(RESEARCH_ROBOT_GATHERER) && building_model_exists(BUILDING_WORKSHOP)) {
			TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "g - gatherer robot");
			TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		}
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "esc - back");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
	} else if (state == MENU_MOVEBUILDING) {
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "Building Placement Menu");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "up - move building");
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "down - move building");
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "left - move building");
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "right - move building");
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "enter - place building");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "esc - back");
	} else if (state == MENU_RESEARCH) {
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "Research Menu");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "g - gatherer");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
		TCOD_console_print_left(NULL, 1, linenum++, TCOD_BKGND_NONE, "esc - back");
		TCOD_console_hline(NULL, 1, linenum++, 28, TCOD_BKGND_NONE);
	}
}
