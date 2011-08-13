#ifndef DWARF_H
#define DWARF_H

#include "point.h"

struct _task;
struct _taskstep;

typedef struct {
	point p;
	int speed;
	int carrying;
	struct _task *curr_task;
	struct _taskstep *curr_taskstep;
} dwarf;

int dwarf_pickup(dwarf *d);
int dwarf_consume(dwarf *d);
void dwarf_act(dwarf *d, float frameduration);

int dwarf_search(dwarf *d, int item);
int dwarf_build(dwarf *d, int model, int x, int y);

#endif
