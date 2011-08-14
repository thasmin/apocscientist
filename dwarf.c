#include "dwarf.h"

#include <stdio.h>
#include <string.h>

#include "task.h"
#include "map.h"
#include "building.h"

void dwarf_act(dwarf *d, float frameduration)
{
	// make sure dwarf has a task
	if (d->curr_task == NULL)
		return;
	d->curr_task->act(d, frameduration);
	return;

	/*
	// see if dwarf has started it yet
	if (d->curr_taskstep == NULL)
		d->curr_taskstep = d->curr_task->steps;

	switch (d->curr_taskstep->steptype) {
		case STEPTYPE_MOVE:
			point_moveto(&d->p, &d->curr_taskstep->details.dest, d->speed * frameduration);
			if (point_equals(&d->p, &d->curr_taskstep->details.dest))
				d->curr_taskstep = d->curr_taskstep->next;
			break;
		case STEPTYPE_ACT:
			d->curr_taskstep->details.act(d);
			d->curr_taskstep = d->curr_taskstep->next;
			break;
		case STEPTYPE_WAIT:
			d->curr_taskstep->details.wait -= frameduration;
			if (d->curr_taskstep->details.wait <= 0)
				d->curr_taskstep = d->curr_taskstep->next;
			break;
	}

	// if we're done and we're not repeating or repeating failed, we're done
	if (d->curr_taskstep == NULL) {
	       	if (!d->curr_task->repeat || d->curr_task->recreate(d) == 0) {
			d->curr_task->destroy(d->curr_task);
			d->curr_task = NULL;
		} else {
			// repeat successful
			d->curr_taskstep = d->curr_task->steps;
		}
	}
	*/
}

int dwarf_pickup(dwarf *d)
{
	int item = map_pickup_item(&d->p);
	if (item == ITEM_NONE)
		return 0;
	d->carrying = item;
	return 1;
}

int dwarf_consume(dwarf *d)
{
	if (d->carrying == ITEM_NONE)
		return 0;
	d->carrying = ITEM_NONE;
	return 1;
}

int task_search_act(dwarf *d, float frameduration)
{
	task *t = d->curr_task;
	point* where = (point*)t->localdata;

	// stage 0 is move
	// stage 1 is pickup
	switch (t->stage) {
		case 0:
			point_moveto(&d->p, where, d->speed * frameduration);
			if (point_equals(&d->p, where))
				t->stage++;
			return 1;
		case 1:
			dwarf_pickup(d);
			return 0;
	}
	return 0;
}

int dwarf_search(dwarf *d, int item)
{
	// can only search for screws now
	if (item != ITEM_SCREW)
		return 0;

	if (d->curr_task != NULL)
		task_destroy(d->curr_task);

	if (d->carrying != ITEM_NONE) {
		map_drop_item(&d->p, d->carrying);
		d->carrying = ITEM_NONE;
	}

	point *where = map_find_closest(&d->p, item);
	d->curr_task = malloc(sizeof(task));
	if (item == ITEM_SCREW)
		d->curr_task->desc = "Search for a screw";
	d->curr_task->act = task_search_act;
	d->curr_task->localdata = malloc(sizeof(point));
	d->curr_task->stage = 0;
	memcpy(d->curr_task->localdata, where, sizeof(point));

	return 1;
}

typedef struct {
	point where;
	int model;
	float delay;
} task_build_data;

int task_build_act(dwarf *d, float frameduration)
{
	task *t = d->curr_task;
	task_build_data* data = (task_build_data*)t->localdata;

	// stage 0 is move
	// stage 1 is delay
	// stage 2 is build
	point center = { data->where.x + 2, data->where.y + 2 };
	switch (t->stage) {
		case 0:
			point_moveto(&d->p, &center, d->speed * frameduration);
			if (point_equals(&d->p, &center))
				t->stage++;
			return 1;
		case 1:
			data->delay -= frameduration;
			if (data->delay <= 0)
				t->stage++;
			return 1;
		case 2:
			building_add(data->model, &data->where);
			return 0;
	}
	return 0;
}

int dwarf_build(dwarf *d, int model, int x, int y)
{
	if (d->curr_task != NULL)
		task_destroy(d->curr_task);

	// TODO: validate building site

	if (d->carrying != ITEM_NONE) {
		map_drop_item(&d->p, d->carrying);
		d->carrying = ITEM_NONE;
	}

	d->curr_task = malloc(sizeof(task));
	d->curr_task->desc = "Build a building";
	d->curr_task->act = task_build_act;
	d->curr_task->stage = 0;
	d->curr_task->localdata = malloc(sizeof(task_build_data));
	task_build_data *data = (task_build_data*)d->curr_task->localdata;
	data->where.x = x;
	data->where.y = y;
	data->model = model;
	data->delay = 2;

	return 1;
}
