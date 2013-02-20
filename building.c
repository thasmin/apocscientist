#include "building.h"

#include <string.h>
#ifdef MACOSX
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
#include <limits.h>
#include <tcod/libtcod.h>

#include "map.h"
#include "robot.h"

building* buildings;

building_model building_models[] = {
	{ BUILDING_LABORATORY, 5, 5, "Lab" },
	{ BUILDING_WORKSHOP,   5, 5, "Wrk" },
	{ BUILDING_STORAGE,    7, 7, "Stg" },
	{ BUILDING_QUARRY,     5, 5, "Qry" },
};

void buildings_init()
{
	building_models[BUILDING_LABORATORY].color = TCOD_yellow;
	building_models[BUILDING_WORKSHOP].color = TCOD_green;
	building_models[BUILDING_STORAGE].color = TCOD_blue;
	building_models[BUILDING_QUARRY].color = TCOD_silver;
	buildings = NULL;
}

void buildings_destroy()
{
	building *b = buildings;
	building *next;
	while (b != NULL) {
		next = b->next;
		free(b);
	}
}

building_model* building_get_model(int model)
{
	return &building_models[model];
}

building* building_create(int model, point *p)
{
	building *b = malloc(sizeof(building));
	b->model = &building_models[model];
	memcpy(&b->p, p, sizeof(point));
	b->worker = NULL;
	b->next = buildings;
	return b;
}

void building_add(int model, point *p)
{
	building *b = building_create(model, p);
	b->next = buildings;
	buildings = b;

	//printf("building size %d at %f, %f\n", b->model->height, b->p.x, b->p.y);
	// make the building not walkable
	for (int i = b->p.x; i <= b->p.x + b->model->width; ++i) {
		//printf("unwalkable: %d, %d\n", i, (int)b->p.y);
		map_set_walkable(i, (int)b->p.y, false);
		//printf("unwalkable: %d, %d\n", i, (int)b->p.y + b->model->height);
		map_set_walkable(i, (int)b->p.y + b->model->height, false);
	}
	for (int i = b->p.y; i <= b->p.y + b->model->height; ++i) {
		//printf("unwalkable: %d, %d\n", (int)b->p.x, i);
		map_set_walkable((int)b->p.x, i, false);
		//printf("unwalkable: %d, %d\n", (int)b->p.x + b->model->width, i);
		map_set_walkable((int)b->p.x + b->model->width, i, false);
	}
	//printf("  walkable: %d, %d\n", (int)b->p.x + b->model->width / 2,
	//								(int)b->p.y + b->model->height);
	map_set_walkable((int)b->p.x + b->model->width / 2,
					 (int)b->p.y + b->model->height, true);
}

void building_draw(building *b)
{
    TCOD_color_t old_color = TCOD_console_get_default_foreground(NULL);
	TCOD_console_set_default_foreground(NULL, b->model->color);
	TCOD_console_print_frame(NULL, b->p.x, b->p.y,
		b->model->width, b->model->height,
		false, TCOD_BKGND_NONE, NULL);
	TCOD_console_print(NULL, b->p.x + 1, b->p.y, b->model->code);
	TCOD_console_set_default_foreground(NULL, old_color);
}

void buildings_draw()
{
	for (building *b = buildings; b != NULL; b = b->next)
		building_draw(b);
}

building* building_find_closest(point *p, int model)
{
	building *closest = NULL;
	float closest_dist = INT_MAX;
	for (building *b = buildings; b != NULL; b = b->next)
	{
		if (b->model->model != model)
			continue;
		float dist = point_dist(&b->p, p);
		if (dist < closest_dist) {
			closest = b;
			closest_dist = dist;
		}
	}
	return closest;
}

void building_adjust_to_center(building *b, point *p)
{
	p->x += b->model->width / 2;
	p->y += b->model->height / 2;
}

int building_model_exists(int model)
{
	for (building *b = buildings; b != NULL; b = b->next)
		if (b->model->model == model)
			return 1;
	return 0;
}

building* building_get_next_of_model(int model, building *start)
{
	building *b = start == NULL ? buildings : start->next;
	while (b != NULL) {
		if (b->model->model == model)
			return b;
		b = b->next;
	}
	return NULL;
}

void building_set_worker(building *building, robot *worker)
{
	building->worker = worker;
}

