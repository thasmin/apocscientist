#ifndef MENU_H
#define MENU_H

typedef enum {
	MENU_NONE,
	MENU_SEARCH,
	MENU_BUILD,
	MENU_MOVEBUILDING,
	MENU_RESEARCH,
	MENU_CHOOSE_ROBOT,
	MENU_ASSIGN_JOB,
} menu_state;

void menu_init();
menu_state menu_get_state();
void menu_set_state(menu_state new_state);
void menu_draw();
void menu_set_message(const char* message);

#endif
