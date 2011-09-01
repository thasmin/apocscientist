#include "robot.h"

#include <stdio.h>
#include <string.h>

#include "task.h"
#include "map.h"
#include "building.h"

robot* robots;
robot genius;

robot_model robot_models[] = {
	{ ROBOT_GENIUS, 12, 2, },
	{ ROBOT_GATHERER, 10, 237, },
};

void robots_init()
{
	genius.model = robot_models[ROBOT_GENIUS];
	genius.p.x = 40;
	genius.p.y = 10;
	robots = &genius;
}

void robot_create(int model, int x, int y)
{
	robot *r = malloc(sizeof(robot));
	r->model = robot_models[model];
	r->p.x = x;
	r->p.y = y;
	r->carrying = ITEM_NONE;
	r->curr_task = NULL;
	robot_add(r);
}

void robot_add(robot *d)
{
	// genius is first
	if (robots->next == NULL) {
		robots->next = d;
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
