#include "task.h"

#include <string.h>

void task_init(task *t, const char *desc)
{
	t->desc = desc;
	t->recreate = NULL;
	t->destroy = task_destroy;
	t->repeat = 0;
	t->steps = NULL;
}

void task_reset(task *t)
{
	task_destroy_steps(t);
	t->steps = NULL;
}

int task_destroy_steps(task *t)
{
	if (t == NULL)
		return 1;
	if (t->steps != NULL) {
		taskstep *step = t->steps;
		while (step) {
			taskstep *next = step->next;
			free(step);
			step = next;
		}
	}
	return 1;
}

int task_destroy(task *t)
{
	if (t == NULL)
		return 1;
	if (task_destroy_steps(t) == 0)
		return 0;
	free(t);
	return 1;
}

void taskstep_append(task *t, taskstep *step)
{
	taskstep* last = t->steps;
	if (last == NULL) {
		t->steps = step;
		return;
	}

	while (last->next != NULL)
		last = last->next;
	last->next = step;
}

taskstep* taskstep_create_move(task *t, const char* desc, point *dest)
{
	if (t == NULL)
		return NULL;

	taskstep* step = malloc(sizeof(taskstep));
	step->steptype = STEPTYPE_MOVE;
	step->desc = desc;
	memcpy(&step->details.dest, dest, sizeof(point));
	step->next = NULL;

	taskstep_append(t, step);

	return step;
}

taskstep* taskstep_create_act(task *t, const char* desc, int (*act)(dwarf*))
{
	if (t == NULL)
		return NULL;

	taskstep* step = malloc(sizeof(taskstep));
	step->steptype = STEPTYPE_ACT;
	step->desc = desc;
	step->details.act = act;
	step->next = NULL;

	taskstep_append(t, step);

	return step;
}

taskstep* taskstep_create_wait(task *t, const char* desc, float secs)
{
	if (t == NULL)
		return NULL;

	taskstep* step = malloc(sizeof(taskstep));
	step->steptype = STEPTYPE_WAIT;
	step->desc = desc;
	step->details.wait = secs;
	step->next = NULL;

	taskstep_append(t, step);

	return step;
}
