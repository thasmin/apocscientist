#ifndef DWARF_H
#define DWARF_H

#include "point.h"

struct _job;

typedef struct _robot {
	int robot_id;
	const char* name;
	int speed;
	int mapchar;
	const struct _job *job;
	point p;
	int carrying;
	struct _task *curr_task;
	struct _robot *next;
} robot;

void robots_init();
void robot_add(robot *d);
robot* robot_create(const char* name, int x, int y);
void robot_set_job(robot *robot, int job);
void robot_reset_idle();
robot* robot_next_idle();
robot* robot_genius();

void robots_act(float frameduration);

int robot_pickup(robot *d);
int robot_drop_item(robot *d);
int robot_consume(robot *d);
void robot_act(robot *d, float frameduration);

int robot_search(robot *d, int item);

#endif
