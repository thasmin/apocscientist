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
	task_reset(d->curr_task);
	taskstep_create_move(d->curr_task, "Moving to a screw", screw);
	taskstep_create_act(d->curr_task, "Picking up a screw", dwarf_pickup);
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
	task_init(t, NULL);
	if (item == ITEM_SCREW)
	{
		t->desc = "Search for a screw";
		t->recreate = search_screw_recreate;
		taskstep_create_move(t, "Moving to a screw", where);
		taskstep_create_act(t, "Picking up a screw", dwarf_pickup);
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

	d->curr_task = malloc(sizeof(task));
	task_init(d->curr_task, "Build a building");
	point p = { x+2, y+2 };
	taskstep_create_move(d->curr_task, "Moving to the building site", &p);
	taskstep_create_wait(d->curr_task, "Building", 3);
	taskstep_create_act(d->curr_task, "Cutting the ribbon", build_building);
	d->curr_taskstep = d->curr_task->steps;

	return 1;
}
