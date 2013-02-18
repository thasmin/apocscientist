#include "job.h"
#include "robot.h"
#include "map.h"
#include "building.h"
#include "lualib.h"

job jobs[JOB_COUNT];

task* job_none_idle(robot *r);
task* job_gatherer_idle(robot *r);
task* job_miner_idle(robot *r);

task* task_mine_create(robot *r);

void job_init()
{
	jobs[JOB_NONE].name = "Jobless";
	jobs[JOB_NONE].mapchar = 2;
	jobs[JOB_NONE].idle_task = job_none_idle;

	jobs[JOB_GATHERER].name = "Gatherer";
	jobs[JOB_GATHERER].mapchar = 237;
	jobs[JOB_GATHERER].idle_task = job_gatherer_idle;

	jobs[JOB_MINER].name = "Miner";
	jobs[JOB_MINER].mapchar = 5;
	jobs[JOB_MINER].idle_task = job_miner_idle;
}

const job* job_get(job_occupation job)
{
	return &jobs[job];
}

task* job_none_idle(robot *r)
{
	return NULL;
}

task* job_gatherer_idle(robot *r)
{
	// see if there are and gatherable items and create a task to pick it up
	if (map_item_list(ITEM_SCRAP) != NULL)
		return task_search_create(ITEM_SCRAP);
	if (map_item_list(ITEM_ROCK) != NULL)
		return task_search_create(ITEM_ROCK);
	return NULL;
}

task* job_miner_idle(robot *r)
{
	building *quarry = NULL;
	while ((quarry = building_get_next_of_model(BUILDING_QUARRY, quarry)) != NULL) {
		if (quarry->worker != NULL)
			continue;
		return task_mine_create(r);
	}
	return 0;
}

typedef struct {
	building *building;
	float delay;
} task_mine_data;

int task_mine_act(robot *d, float frameduration)
{
	task *t = d->curr_task;
	task_mine_data* data = (task_mine_data*)t->localdata;

	// stage 0 is move
	// stage 1 is delay
	// stage 2 is build
	building *building = data->building;
	point center = {
		data->building->p.x + building->model->width / 2,
		data->building->p.y + building->model->height / 2
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
			map_create_item(center.x, center.y + 3, ITEM_ROCK);
			return 0;
	}
	return 0;
}

task* task_mine_create(robot *r)
{
	// load tasks, get the item at index 0, then get the create method
	lua_getglobal(L, "tasks");
	lua_pushinteger(L, 1);
	lua_gettable(L, -2);
	lua_pushstring(L, "create");
	lua_gettable(L, -2);

	// push a robot onto the stack and call the function
	lh_push_robot(L, robot_genius());
	int err = lua_pcall(L, 1, 1, 0);
	if (err != 0)
		printf("error: %s\n", lua_tostring(L, -1));
	lua_pushstring(L, "act");
	lua_gettable(L, -2);
	printf("top of lua stack is a %s\n", luaL_typename(L, -1));

	//return;
	lua_pop(L, 3);

	task *t = malloc(sizeof(task));
	t->desc = "Mine the quarry";
	t->act = task_mine_act;
	t->stage = 0;

	t->localdata = malloc(sizeof(task_mine_data));
	t->localdata_size = sizeof(task_mine_data);
	task_mine_data *data = (task_mine_data*) t->localdata;
	// TODO: find a quarry without a worker
	data->building = building_find_closest(&r->p, BUILDING_QUARRY);
	data->delay = 2;
	return t;
}


