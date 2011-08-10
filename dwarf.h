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

int dwarf_pickup(dwarf *d, int item);
void dwarf_act(dwarf *d);

void dwarf_search(dwarf *d, int item);

#endif
