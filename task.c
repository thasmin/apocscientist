#include "task.h"
#include "building.h"
#include "map.h"

#include <string.h>

int task_destroy(task *t)
{
	if (t == NULL)
		return 1;
	free(t->localdata);
	free(t);
	return 1;
}

typedef struct {
	int item;
	point where;
} task_search_data;

int task_search_act(dwarf *d, float frameduration)
{
	task *t = d->curr_task;
	task_search_data* data = (task_search_data*)t->localdata;

	// stage 0 is move
	// stage 1 is pickup
	switch (t->stage) {
		case 0:
			point_moveto(&d->p, &data->where, d->speed * frameduration);
			if (point_equals(&d->p, &data->where))
				t->stage++;
			return 1;
		case 1:
			dwarf_pickup(d);
			return 0;
	}
	return 0;
}

int task_search_assign(task *t, dwarf *d)
{
	task_search_data *taskdata = (task_search_data*)t->localdata;
	point *where = map_find_closest(&d->p, taskdata->item);
	memcpy(&taskdata->where, where, sizeof(point));

	dwarf_drop_item(d);

	return 1;
}

task* task_search_create(int item)
{
	task *t = malloc(sizeof(task));
	if (item == ITEM_SCREW)
		t->desc = "Search for a screw";
	t->assign = task_search_assign;
	t->act = task_search_act;
	t->stage = 0;
	t->localdata = malloc(sizeof(task_search_data));
	task_search_data *data = (task_search_data*)t->localdata;
	data->item = item;
	return t;
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

task* task_build_create(int model, int x, int y)
{
	task *t = malloc(sizeof(task));
	t->desc = "Build a building";
	t->assign = NULL;
	t->act = task_build_act;
	t->stage = 0;
	t->localdata = malloc(sizeof(task_build_data));
	task_build_data *data = (task_build_data*)t->localdata;
	data->where.x = x;
	data->where.y = y;
	data->model = model;
	data->delay = 2;
	return t;
}
