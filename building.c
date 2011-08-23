#include "building.h"

#include <string.h>
#include <malloc.h>
#include <limits.h>

building* buildings;

building_model building_models[] = {
	{ BUILDING_LABORATORY, 5, 5, "Lab" },
	{ BUILDING_WORKSHOP,   5, 5, "Wrk" },
	{ BUILDING_STORAGE,    7, 7, "Stg" },
};

void buildings_init()
{
	building_models[BUILDING_LABORATORY].color = TCOD_yellow;
	building_models[BUILDING_WORKSHOP].color = TCOD_green;
	building_models[BUILDING_STORAGE].color = TCOD_blue;
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
	b->next = buildings;
	return b;
}

void building_add(int model, point *p)
{
	building *b = building_create(model, p);
	b->next = buildings;
	buildings = b;
}

void building_draw(building *b)
{
	TCOD_color_t old_color = TCOD_console_get_foreground_color(NULL);
	TCOD_console_set_foreground_color(NULL, b->model->color);
	TCOD_console_print_frame(NULL, b->p.x, b->p.y,
		b->model->width, b->model->height,
		false, TCOD_BKGND_NONE, NULL);
	TCOD_console_print_left(NULL, b->p.x + 1, b->p.y, TCOD_BKGND_NONE, b->model->code);
	TCOD_console_set_foreground_color(NULL, old_color);
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

// warning: returning a struct by value
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
