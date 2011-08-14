#ifndef TASK_H
#define TASK_H

#include "point.h"
#include "dwarf.h"

typedef struct _task {
	const char *desc;
	int (*act)(dwarf*, float);
	int stage;
	void* localdata;
} task;

int task_destroy(task *t);

#endif
