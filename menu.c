#include "menu.h"

#include <stdio.h>
#include <tcod/libtcod.h>

#include "dwarf.h"
#include "task.h"

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
	TCOD_console_print_frame(NULL, 0, 0, 30, 50, false, TCOD_BKGND_NONE, NULL);
	if (state == MENU_NONE)
	{
		TCOD_console_print_left(NULL, 1, 1, TCOD_BKGND_NONE, "Menu");
		TCOD_console_hline(NULL, 1, 2, 28, TCOD_BKGND_NONE);
		TCOD_console_print_left(NULL, 1, 3, TCOD_BKGND_NONE, "b - build");
		TCOD_console_print_left(NULL, 1, 4, TCOD_BKGND_NONE, "s - search");
		TCOD_console_print_left(NULL, 1, 5, TCOD_BKGND_NONE, "q - quit");
		TCOD_console_hline(NULL, 1, 6, 28, TCOD_BKGND_NONE);
		TCOD_console_print_left(NULL, 1, 7, TCOD_BKGND_NONE, "The dwarf is...");
		if (guy.curr_taskstep != NULL)
			TCOD_console_print_left(NULL, 3, 8, TCOD_BKGND_NONE, guy.curr_taskstep->desc);
		else
			TCOD_console_print_left(NULL, 3, 8, TCOD_BKGND_NONE, "doing nothing");
	} else if (state == MENU_SEARCH) {
		TCOD_console_print_left(NULL, 1, 1, TCOD_BKGND_NONE, "Search Menu");
		TCOD_console_hline(NULL, 1, 2, 28, TCOD_BKGND_NONE);
		TCOD_console_print_left(NULL, 1, 3, TCOD_BKGND_NONE, "s - screws");
		TCOD_console_print_left(NULL, 1, 4, TCOD_BKGND_NONE, "esc - back");
		TCOD_console_hline(NULL, 1, 5, 28, TCOD_BKGND_NONE);
	} else if (state == MENU_BUILD) {
		TCOD_console_print_left(NULL, 1, 1, TCOD_BKGND_NONE, "Build Menu");
		TCOD_console_hline(NULL, 1, 2, 28, TCOD_BKGND_NONE);
		TCOD_console_print_left(NULL, 1, 3, TCOD_BKGND_NONE, "l - laboratory");
		TCOD_console_print_left(NULL, 1, 4, TCOD_BKGND_NONE, "w - workshop");
		TCOD_console_print_left(NULL, 1, 5, TCOD_BKGND_NONE, "esc - back");
		TCOD_console_hline(NULL, 1, 6, 28, TCOD_BKGND_NONE);
	} else if (state == MENU_MOVEBUILDING) {
		TCOD_console_print_left(NULL, 1, 1, TCOD_BKGND_NONE, "Building Placement Menu");
		TCOD_console_hline(NULL, 1, 2, 28, TCOD_BKGND_NONE);
		TCOD_console_print_left(NULL, 1, 3, TCOD_BKGND_NONE, "up - move building");
		TCOD_console_print_left(NULL, 1, 4, TCOD_BKGND_NONE, "down - move building");
		TCOD_console_print_left(NULL, 1, 5, TCOD_BKGND_NONE, "left - move building");
		TCOD_console_print_left(NULL, 1, 6, TCOD_BKGND_NONE, "right - move building");
		TCOD_console_print_left(NULL, 1, 7, TCOD_BKGND_NONE, "enter - place building");
		TCOD_console_hline(NULL, 1, 8, 28, TCOD_BKGND_NONE);
		TCOD_console_print_left(NULL, 1, 9, TCOD_BKGND_NONE, "esc - back");
	}
}
