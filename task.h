#ifndef TASK_H
#define TASK_H

#include "point.h"
#include "dwarf.h"

#define RESEARCH_ROBOT_GATHERER	0
#define RESEARCH_COUNT		1

typedef struct _task {
	const char *desc;
	int (*act)(dwarf*, float);
	int stage;
	void* localdata;
} task;

int task_destroy(task *t);

task* task_search_create(int item);
task* task_build_create(int model_id, int x, int y);
task* task_research_create(int robot);

void research_init();
int research_is_completed(int research_id);

#endif
