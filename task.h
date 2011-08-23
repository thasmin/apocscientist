#ifndef TASK_H
#define TASK_H

#include "point.h"
#include "robot.h"

#define ITEM_NONE 	0
#define ITEM_BUCKET 	1
#define ITEM_WELL	2
#define ITEM_SCREW	3
#define ITEM_COUNT	4

#define RESEARCH_ROBOT_GATHERER	0
#define RESEARCH_COUNT		1

#define BUILDABLE_ROBOT_GATHERER	0
#define BUILDABLE_COUNT			1

typedef struct _task {
	const char *desc;
	int (*act)(robot*, float);
	int stage;
	void* localdata;
} task;

int task_destroy(task *t);

task* task_search_create(int item);
task* task_construct_create(int model_id, int x, int y);
task* task_research_create(int robot);
task* task_build_create(int buildable);

void research_init();
int research_is_completed(int research_id);

#endif
