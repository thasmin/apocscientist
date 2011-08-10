#include "task.h"

#include <string.h>

int task_destroy(task *t)
{
	if (t->steps != NULL) {
		taskstep *step = t->steps;
		while (step) {
			taskstep *next = step->next;
			free(step);
			step = next;
		}
	}
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
