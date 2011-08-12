#include "dwarf.h"

#include <stdio.h>
#include <string.h>
#include <tcod/libtcod.h>

#include "task.h"
#include "map.h"
#include "building.h"

void dwarf_act(dwarf *d)
{
	// make sure dwarf has a task
	if (d->curr_task == NULL)
		return;

	// see if dwarf has started it yet
	if (d->curr_taskstep == NULL)
		d->curr_taskstep = d->curr_task->steps;

	// see if dwarf has arrived
	if (point_equals(&d->p, &d->curr_taskstep->dest)) {
		d->curr_taskstep->act(d);
		d->curr_taskstep = d->curr_taskstep->next;
		if (d->curr_taskstep == NULL) {
			// if it's repeated try to recreate the task
			if (!d->curr_task->repeat ||
			    d->curr_task->recreate == NULL ||
			    d->curr_task->recreate(d) == 0) {
				d->curr_task->destroy(d->curr_task);
				d->curr_task = NULL;
			}
		}
	} else {
		point_moveto(&d->p, &d->curr_taskstep->dest, d->speed * TCOD_sys_get_last_frame_length());
	}
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

int search_screw_recreate(dwarf *d)
{
	point* screw = map_find_closest(&d->p, ITEM_SCREW);
	if (screw == NULL)
		return 0;
	task_destroy_steps(d->curr_task);
	d->curr_task->steps = taskstep_create("Searching for a screw", screw, dwarf_pickup);
	return 1;
}

int dwarf_search(dwarf *d, int item)
{
	// can only search for screws now
	if (item != ITEM_SCREW)
		return 0;

	if (d->curr_task != NULL)
		d->curr_task->destroy(d->curr_task);

	if (d->carrying != ITEM_NONE) {
		map_drop_item(&d->p, d->carrying);
		d->carrying = ITEM_NONE;
	}

	point *where = map_find_closest(&d->p, item);
	task* t = malloc(sizeof(task));
	t->destroy = task_destroy;
	if (item == ITEM_SCREW)
	{
		t->desc = "Search for a screw";
		t->recreate = search_screw_recreate;
		t->steps = taskstep_create("Searching for a screw", where, dwarf_pickup);
	}
	d->curr_task = t;
	d->curr_taskstep = t->steps;

	return 1;
}

int tobuild_model, tobuild_x, tobuild_y;
int build_building(dwarf *d)
{
	building_add(tobuild_model, tobuild_x, tobuild_y);
	return 1;
}

int dwarf_build(dwarf *d, int model, int x, int y)
{
	if (d->curr_task != NULL)
		d->curr_task->destroy(d->curr_task);

	// TODO: validate building site

	if (d->carrying != ITEM_NONE) {
		map_drop_item(&d->p, d->carrying);
		d->carrying = ITEM_NONE;
	}

	tobuild_model = model;
	tobuild_x = x;
	tobuild_y = y;

	task* t = malloc(sizeof(task));
	t->desc = "Build a building";
	t->recreate = NULL;
	t->destroy = task_destroy;
	t->repeat = 0;
	point p = { x+2, y+2 };
	t->steps = taskstep_create("Searching for a screw", &p, build_building);
	d->curr_task = t;
	d->curr_taskstep = d->curr_task->steps;

	return 1;
}
