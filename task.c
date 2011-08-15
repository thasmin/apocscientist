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
	point item_p;
	building* storage;
	point storage_p;
} task_search_data;

int task_search_act(dwarf *d, float frameduration)
{
	task *t = d->curr_task;
	task_search_data* data = (task_search_data*)t->localdata;

	// stage 0 is move
	// stage 1 is pickup
	// stage 2 is move to storage
	// stage 3 is add item to storage
	switch (t->stage) {
		case 0:
			point_moveto(&d->p, &data->item_p, d->speed * frameduration);
			if (point_equals(&d->p, &data->item_p))
				t->stage++;
			return 1;
		case 1:
			dwarf_pickup(d);
			data->storage = building_find_closest(&data->item_p, BUILDING_STORAGE);
			if (data->storage == NULL)
				return 0;
			memcpy(&data->storage_p, &data->storage->p, sizeof(point));
			building_adjust_to_center(data->storage, &data->storage_p);
			t->stage++;
			return 1;
		case 2:
			point_moveto(&d->p, &data->storage_p, d->speed * frameduration);
			if (point_equals(&d->p, &data->storage_p))
				t->stage++;
			return 1;
		case 3:
			storage_add(data->item);
			dwarf_consume(d);
			return 0;
	}
	return 0;
}

int task_search_assign(task *t, dwarf *d)
{
	task_search_data *taskdata = (task_search_data*)t->localdata;
	point *where = map_find_closest(&d->p, taskdata->item);
	memcpy(&taskdata->item_p, where, sizeof(point));

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
	building_model *model = building_get_model(data->model);
	point center = {
		data->where.x + model->width / 2,
		data->where.y + model->height / 2
	};
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
