#ifndef ORDERS_H
#define ORDERS_H

#include "task.h"
#include "robot.h"

typedef struct _order {
	task* task;
	struct _order *next;
} order;

void orders_init();
void order_add(task* t);
order* orders_list();
task* order_next(robot *d);

#endif
