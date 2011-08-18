#ifndef DWARF_H
#define DWARF_H

#include "point.h"

struct _task;

#define ROBOT_GENIUS	0
#define ROBOT_GATHERER	1

typedef struct _dwarf {
	point p;
	int speed;
	int carrying;
	struct _task *curr_task;
	struct _dwarf *next;
} dwarf;

void dwarves_init();
void dwarf_add(dwarf *d);
void dwarf_reset_idle();
dwarf* dwarf_next_idle();
void dwarves_act(float frameduration);

int dwarf_pickup(dwarf *d);
int dwarf_drop_item(dwarf *d);
int dwarf_consume(dwarf *d);
void dwarf_act(dwarf *d, float frameduration);

int dwarf_search(dwarf *d, int item);

#endif
