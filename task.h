#ifndef TASK_H
#define TASK_H

#include "point.h"
#include "dwarf.h"

typedef struct _taskstep {
	const char *desc;
	point dest;
	int (*act)(dwarf*);
	struct _taskstep *next;
} taskstep;

typedef struct _task {
	const char *desc;
	int (*recreate)(dwarf*);
	int (*destroy)(struct _task*);
	taskstep *steps;
	int repeat;
} task;

int task_destroy_steps(task *t);
int task_destroy(task *t);

taskstep* taskstep_create(const char *desc, point *dest, int (*act)(dwarf*));

#endif
