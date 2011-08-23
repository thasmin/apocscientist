#ifndef DWARF_H
#define DWARF_H

#include "point.h"

struct _task;

#define ROBOT_GENIUS	0
#define ROBOT_GATHERER	1

typedef struct _robot_model {
	int robot_id;
	int speed;
	int mapchar;
} robot_model;

typedef struct _robot {
	robot_model model;
	point p;
	int carrying;
	struct _task *curr_task;
	struct _robot *next;
} robot;

void robots_init();
void robot_add(robot *d);
void robot_create(int model, int x, int y);
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
