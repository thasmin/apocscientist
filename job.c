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

task* task_mine_create(robot *r)
{
	// load tasks, get the mine task and call it with a robot on the stack
	lua_getglobal(L, "tasks");
	lua_pushstring(L, "mine");
	lua_gettable(L, -2);
	lh_push_robot(L, robot_genius());
	int err = lua_pcall(L, 1, 1, 0);
	if (err != 0)
		printf("error: %s\n", lua_tostring(L, -1));

	// get the description
	task *t = malloc(sizeof(task));
	lua_pushstring(L, "desc");
	lua_gettable(L, -2);
	t->desc = luaL_checkstring(L, -1);
	lua_pop(L, 1);

	// store the lua table so we can call act later
	t->lua_table = luaL_ref(L, LUA_REGISTRYINDEX);
	t->act = NULL;

	lua_pop(L, 1);
	return t;
}


