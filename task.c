#include "task.h"

#include <string.h>

int task_destroy(task *t)
{
	if (t == NULL)
		return 1;
	free(t->localdata);
	free(t);
	return 1;
}

