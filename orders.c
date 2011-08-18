#include "orders.h"

order* pending;

void orders_init()
{
	pending = NULL;
}

void order_add(task* t)
{
	order *o = malloc(sizeof(order));
	o->task = t;

	if (pending == NULL) {
		o->next = NULL;
		pending = o;
	} else {
		o->next = pending;
		pending = o;
	}
}

order* orders_list()
{
	return pending;
}

task* order_next(dwarf *d)
{
	if (pending == NULL)
		return NULL;

	order *togive = pending;
	pending = pending->next;
	return togive->task;
}

