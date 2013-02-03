#include "robot.h"

#include <stdio.h>
#include <string.h>

#include "task.h"
#include "map.h"
#include "building.h"
#include "job.h"

robot* robots;

void robots_init()
{
	robot_create("robot 1", 40, 10);
	robot_create("robot 2", 44, 13);
	robot_create("robot 3", 46, 12);
}

robot* robot_create(const char* name, int x, int y)
{
	robot *r = malloc(sizeof(robot));
	r->name = name;
	r->p.x = x;
	r->p.y = y;
	r->speed = 6;
	r->carrying = ITEM_NONE;
	r->curr_task = NULL;
	r->job = job_get(JOB_NONE);
	robot_add(r);
	return r;
}

void robot_set_job(robot *robot, int job)
{
	robot->job = job_get(job);
}

void robot_add(robot *d)
{
	if (robots == NULL) {
		robots = d;
		d->next = NULL;
		return;
	}

	d->next = robots->next;
	robots->next = d;
}

robot *idleptr;
void robot_reset_idle()
{
	idleptr = robots;
}

robot* robot_next_idle()
{
	robot *d;
	for (d = idleptr; d != NULL; d = d->next) {
		idleptr = idleptr->next;
		if (d->curr_task == NULL)
			return d;
	}
	return NULL;
}

robot* robot_genius()
{
	return robots;
}

void robots_act(float frameduration)
{
	robot *d;
	for (d = robots; d != NULL; d = d->next)
		robot_act(d, frameduration);
}

void robot_act(robot *d, float frameduration)
{
	// make sure robot has a task
	if (d->curr_task == NULL)
		return;
	if (d->curr_task->act(d, frameduration) == 0)
		d->curr_task = NULL;
}

int robot_pickup(robot *d)
{
	int item = map_pickup_item(&d->p);
	if (item == ITEM_NONE)
		return 0;
	d->carrying = item;
	return 1;
}

int robot_drop_item(robot *d)
{
	if (d->carrying == ITEM_NONE)
		return 0;
	map_drop_item(&d->p, d->carrying);
	d->carrying = ITEM_NONE;
	return 1;
}

int robot_consume(robot *d)
{
	if (d->carrying == ITEM_NONE)
		return 0;
	d->carrying = ITEM_NONE;
	return 1;
}
