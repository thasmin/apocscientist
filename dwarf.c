#include "dwarf.h"

#include <stdio.h>
#include <string.h>

#include "task.h"
#include "map.h"
#include "building.h"

dwarf* dwarves;

void dwarves_init()
{
	dwarves = NULL;
}

void dwarf_add(dwarf *d)
{
	if (dwarves == NULL) {
		dwarves = d;
		d->next = NULL;
		return;
	}

	d->next = dwarves;
	dwarves = d;
}

dwarf *idleptr;
void dwarf_reset_idle()
{
	idleptr = dwarves;
}

dwarf* dwarf_next_idle()
{
	dwarf *d;
	for (d = idleptr; d != NULL; d = d->next) {
		idleptr = idleptr->next;
		if (d->curr_task == NULL)
			return d;
	}
	return NULL;
}

void dwarves_act(float frameduration)
{
	dwarf *d;
	for (d = dwarves; d != NULL; d = d->next)
		dwarf_act(d, frameduration);
}

void dwarf_act(dwarf *d, float frameduration)
{
	// make sure dwarf has a task
	if (d->curr_task == NULL)
		return;
	if (d->curr_task->act(d, frameduration) == 0)
		d->curr_task = NULL;
}

int dwarf_pickup(dwarf *d)
{
	int item = map_pickup_item(&d->p);
	if (item == ITEM_NONE)
		return 0;
	d->carrying = item;
	return 1;
}

int dwarf_drop_item(dwarf *d)
{
	if (d->carrying == ITEM_NONE)
		return 0;
	map_drop_item(&d->p, d->carrying);
	d->carrying = ITEM_NONE;
	return 1;
}

int dwarf_consume(dwarf *d)
{
	if (d->carrying == ITEM_NONE)
		return 0;
	d->carrying = ITEM_NONE;
	return 1;
}
