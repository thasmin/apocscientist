#include "dwarf.h"

#include <stdio.h>
#include <string.h>
#include <tcod/libtcod.h>

#include "task.h"
#include "map.h"

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
			// hold the create callback for repeated tasks
			int repeat = d->curr_task->repeat;
			task* (*recreate)(dwarf*) = d->curr_task->create;
			d->curr_task->destroy(d->curr_task);
			d->curr_task = NULL;
			// recreate the task if it is repeated
			if (repeat) {
				d->curr_task = recreate(d);
				if (d->curr_task != NULL) {
					d->curr_task->repeat = repeat;
					d->curr_taskstep = d->curr_task->steps;
				}
			}
		}
	} else {
		point_moveto(&d->p, &d->curr_taskstep->dest, d->speed * TCOD_sys_get_last_frame_length());
	}
}

int dwarf_pickup(dwarf *d, int item)
{
	d->carrying = item;
	map_pickup_item(&d->p, item);
	return 1;
}

int get_screw(dwarf *d)
{
	return dwarf_pickup(d, ITEM_SCREW);
}

task* task_search_screw_create(dwarf *d);
task search_screw_task = { "Search for a screw", task_search_screw_create, task_destroy };
task* task_search_screw_create(dwarf *d)
{
	point* screw = map_find_closest(&d->p, ITEM_SCREW);
	if (screw == NULL)
		return NULL;

	if (d->carrying != ITEM_NONE) {
		map_drop_item(&d->p, d->carrying);
		d->carrying = ITEM_NONE;
	}

	task* t = malloc(sizeof(task));
	memcpy(t, &search_screw_task, sizeof(task));
	t->steps = taskstep_create("Searching for a screw", screw, get_screw);
	return t;
}


void dwarf_search(dwarf *d, int item)
{
	if (item == ITEM_SCREW)
	{
		if (d->curr_task != NULL)
			d->curr_task->destroy(d->curr_task);
		d->curr_task = search_screw_task.create(d);
		d->curr_taskstep = d->curr_task->steps;
	}
}


