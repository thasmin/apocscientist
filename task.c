#include "task.h"
#include "building.h"
#include "map.h"

#include <string.h>

int research[RESEARCH_COUNT];

void research_init()
{
	for (int i = 0; i < RESEARCH_COUNT; ++i)
		research[i] = 0;
}

int research_is_completed(int research_id)
{
	return research[research_id];
}

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

int task_search_act(robot *d, float frameduration)
{
	task *t = d->curr_task;
	task_search_data* data = (task_search_data*)t->localdata;

	// stage 0 is find closest item and fall through
	// stage 1 is move
	// stage 2 is pickup
	// stage 3 is move to storage
	// stage 4 is add item to storage
	if (t->stage == 0)
	{
		point *where = map_find_closest(&d->p, data->item);
		memcpy(&data->item_p, where, sizeof(point));
		robot_drop_item(d);
		t->stage++;
	}
	switch (t->stage) {
		case 1:
			point_moveto(&d->p, &data->item_p, d->speed * frameduration);
			if (point_equals(&d->p, &data->item_p))
				t->stage++;
			return 1;
		case 2:
			robot_pickup(d);
			data->storage = building_find_closest(&data->item_p, BUILDING_STORAGE);
			if (data->storage == NULL)
				return 0;
			memcpy(&data->storage_p, &data->storage->p, sizeof(point));
			building_adjust_to_center(data->storage, &data->storage_p);
			t->stage++;
			return 1;
		case 3:
			point_moveto(&d->p, &data->storage_p, d->speed * frameduration);
			if (point_equals(&d->p, &data->storage_p))
				t->stage++;
			return 1;
		case 4:
			storage_add(data->item);
			robot_consume(d);
			return 0;
	}
	return 0;
}

task* task_search_create(int item)
{
	task *t = malloc(sizeof(task));
	if (item == ITEM_SCREW)
		t->desc = "Search for a screw";
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

int task_build_act(robot *d, float frameduration)
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

task* task_build_create(int model_id, int x, int y)
{
	task *t = malloc(sizeof(task));
	t->desc = "Build a building";
	t->act = task_build_act;
	t->stage = 0;
	t->localdata = malloc(sizeof(task_build_data));
	task_build_data *data = (task_build_data*)t->localdata;
	data->where.x = x;
	data->where.y = y;
	data->model = model_id;
	data->delay = 2;
	return t;
}

typedef struct {
	int research;
	point lab_p;
	int research_time;
} task_research_data;

int task_research_act(robot *d, float frameduration)
{
	task *t = d->curr_task;
	task_research_data* data = (task_research_data*)t->localdata;

	// stage 0 is find nearest lab and fall through
	// stage 1 is move to lab
	// stage 2 is delay while researching
	// stage 3 is mark research as complete
	if (t->stage == 0)
	{
		building *lab = building_find_closest(&d->p, BUILDING_LABORATORY);
		memcpy(&data->lab_p, &lab->p, sizeof(point));
		building_adjust_to_center(lab, &data->lab_p);
		t->stage++;
	}
	switch (t->stage)
	{
		case 1:
			point_moveto(&d->p, &data->lab_p, d->speed * frameduration);
			if (point_equals(&d->p, &data->lab_p))
				t->stage++;
			return 1;
		case 2:
			data->research_time -= frameduration;
			if (data->research_time <= 0)
				t->stage++;
			return 1;
		case 3:
			research[data->research] = 1;
			return 0;
	}
	return 0;
}

task* task_research_create(int research)
{
	task *t = malloc(sizeof(task));
	t->desc = "research";
	t->act = task_research_act;
	t->stage = 0;
	t->localdata = malloc(sizeof(task_research_data));
	task_research_data *data = (task_research_data*)t->localdata;
	data->research = research;
	data->research_time = 3;
	return t;
}
