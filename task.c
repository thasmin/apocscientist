#include "task.h"
#include "building.h"
#include "map.h"

#include <string.h>

int research[RESEARCH_COUNT];

int ingredients_robot_gatherer[ITEM_COUNT] = { 0, 0, 0, 2 };
// array of integer pointers called ingredients
int *ingredients[BUILDABLE_COUNT];

void research_init()
{
	for (int i = 0; i < RESEARCH_COUNT; ++i)
		research[i] = 0;

	ingredients[BUILDABLE_ROBOT_GATHERER] = ingredients_robot_gatherer;
}

int research_is_completed(int research_id)
{
	return research[research_id];
}

task* task_clone(task *t)
{
	if (t == NULL)
		return NULL;
	task* n = malloc(sizeof(*t));
	memcpy(n, t, sizeof(*t));
	n->localdata = malloc(t->localdata_size);
	memcpy(n->localdata, t->localdata, t->localdata_size);
	return n;
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
	TCOD_path_t path;
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
		if (where == NULL)
			return 0;
		memcpy(&data->item_p, where, sizeof(point));
		data->path = map_computepath(&d->p, &data->item_p);
		robot_drop_item(d);
		t->stage++;
	}
	switch (t->stage) {
		case 1:
			map_walk(data->path, &d->p, d->model.speed * frameduration);
			//point_moveto(&d->p, &data->item_p, d->model.speed * frameduration);
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
			data->path = map_computepath(&d->p, &data->storage_p);
			t->stage++;
			return 1;
		case 3:
			map_walk(data->path, &d->p, d->model.speed * frameduration);
			//point_moveto(&d->p, &data->storage_p, d->model.speed * frameduration);
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
	if (item == ITEM_SCRAP)
		t->desc = "Search for scrap";
	else
		t->desc = "Searching";
	t->act = task_search_act;
	t->stage = 0;
	t->localdata = malloc(sizeof(task_search_data));
	t->localdata_size = sizeof(task_search_data);
	task_search_data *data = (task_search_data*)t->localdata;
	data->item = item;
	return t;
}

typedef struct {
	point where;
	int model;
	float delay;
} task_construct_data;

int task_construct_act(robot *d, float frameduration)
{
	task *t = d->curr_task;
	task_construct_data* data = (task_construct_data*)t->localdata;

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
			point_moveto(&d->p, &center, d->model.speed * frameduration);
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

task* task_construct_create(int model_id, int x, int y)
{
	task *t = malloc(sizeof(task));
	t->desc = "Build a building";
	t->act = task_construct_act;
	t->stage = 0;
	t->localdata = malloc(sizeof(task_construct_data));
	t->localdata_size = sizeof(task_construct_data);
	task_construct_data *data = (task_construct_data*)t->localdata;
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
			point_moveto(&d->p, &data->lab_p, d->model.speed * frameduration);
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
	t->desc = "Research";
	t->act = task_research_act;
	t->stage = 0;
	t->localdata = malloc(sizeof(task_research_data));
	t->localdata_size = sizeof(task_research_data);
	task_research_data *data = (task_research_data*)t->localdata;
	data->research = research;
	data->research_time = 3;
	return t;
}

typedef struct {
	int buildable;
	int ingredients_remaining[ITEM_COUNT];
	int ingredient_to_take;
	point ingredient_p;
	point workshop_p;
	int build_time;
} task_build_data;

int task_build_act(robot *d, float frameduration)
{
	task *t = d->curr_task;
	task_build_data* data = (task_build_data*)t->localdata;

	// stage 0 is create ingredient list
	// stage 1 is see if all ingredients are present and if so, go to step 6
	// stage 2 is move to storage
	// stage 3 is remove ingredient from storage
	// stage 4 is move to workshop
	// stage 5 is drop ingredient and go to stage 1
	// stage 6 is delay while building
	// stage 7 is create item
	// stage 8 is specific to item?
	if (t->stage == 0) {
		memcpy(data->ingredients_remaining, ingredients[data->buildable], sizeof(int) * ITEM_COUNT);
		t->stage++;
	}
	if (t->stage == 1) {
		for (int i = 0; i < ITEM_COUNT; ++i)
			if (data->ingredients_remaining[i] > 0) {
				data->ingredient_to_take = i;
				building *storage = building_find_closest(&d->p, BUILDING_STORAGE);
				memcpy(&data->ingredient_p, &storage->p, sizeof(point));
				building_adjust_to_center(storage, &data->ingredient_p);
				t->stage = 2;
				return 1;
			}

		// no more ingredients to fetch
		t->stage = 6;
		return 1;
	}
	if (t->stage == 2) {
		point_moveto(&d->p, &data->ingredient_p, d->model.speed * frameduration);
		if (point_equals(&d->p, &data->ingredient_p))
			t->stage++;
		return 1;
	}
	if (t->stage == 3) {
		storage_take(data->ingredient_to_take);
		// if no warehouse has been picked, find the closest
		if (!point_is_valid(&data->workshop_p)) {
			building *workshop = building_find_closest(&d->p, BUILDING_WORKSHOP);
			memcpy(&data->workshop_p, &workshop->p, sizeof(point));
			building_adjust_to_center(workshop, &data->workshop_p);
		}
		t->stage++;
		return 1;
	}
	if (t->stage == 4) {
		point_moveto(&d->p, &data->workshop_p, d->model.speed * frameduration);
		if (point_equals(&d->p, &data->workshop_p))
			t->stage++;
		return 1;
	}
	if (t->stage == 5) {
		data->ingredients_remaining[data->ingredient_to_take] -= 1;
		// check ingredient list again
		t->stage = 1;
		return 1;
	}
	if (t->stage == 6) {
		data->build_time -= frameduration;
		if (data->build_time <= 0)
			t->stage++;
		return 1;
	}
	if (t->stage == 7) {
		robot_create(ROBOT_GATHERER, data->workshop_p.x + 1, data->workshop_p.y + 1);
		return 0;
	}

	return 0;
}

task* task_build_create(int buildable)
{
	task *t = malloc(sizeof(task));
	t->desc = "Build in the workshop";
	t->act = task_build_act;
	t->stage = 0;
	t->localdata = malloc(sizeof(task_build_data));
	t->localdata_size = sizeof(task_build_data);
	task_build_data *data = (task_build_data*)t->localdata;
	data->buildable = buildable;
	point_invalidate(&data->workshop_p);
	data->build_time = 3;
	return t;
}
