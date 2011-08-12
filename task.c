#include "task.h"

#include <string.h>

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

taskstep* taskstep_create(const char *desc, point *dest, int (*act)(dwarf*))
{
	taskstep* step = malloc(sizeof(taskstep));
	step->desc = desc;
	memcpy(&step->dest, dest, sizeof(point));
	step->act = act;
	step->next = NULL;
	return step;
}
