#ifndef TASK_H
#define TASK_H

#include "point.h"
#include "dwarf.h"

typedef struct _task {
	const char *desc;
	int (*assign)(struct _task*, dwarf*);
	int (*act)(dwarf*, float);
	int stage;
	void* localdata;
} task;

int task_destroy(task *t);

task* task_search_create(int item);
task* task_build_create(int model, int x, int y);

#endif
