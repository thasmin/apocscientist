#ifndef TASK_H
#define TASK_H

#include "point.h"
#include "dwarf.h"

#define STEPTYPE_MOVE 0
#define STEPTYPE_ACT 1
#define STEPTYPE_WAIT 2

typedef struct _taskstep {
	const char *desc;
	int steptype;
	union {
		point dest;
		int (*act)(dwarf*);
		float wait;
	} details;
	struct _taskstep *next;
} taskstep;

typedef struct _task {
	const char *desc;
	int (*recreate)(dwarf*);
	int (*destroy)(struct _task*);
	taskstep *steps;
	int repeat;
} task;

void task_init(task *t, const char *desc);
void task_reset(task *t);
int task_destroy_steps(task *t);
int task_destroy(task *t);

taskstep* taskstep_create_wait(task *t, const char* desc, float secs);
taskstep* taskstep_create_act(task *t, const char* desc, int (*act)(dwarf*));
taskstep* taskstep_create_move(task *t, const char* desc, point *dest);

#endif
