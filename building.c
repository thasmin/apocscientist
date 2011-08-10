#include "building.h"

#include <malloc.h>

building* buildings;

building_model models[] = {
	{ BUILDING_LABORATORY, 5, 5, "Lab" },
	{ BUILDING_WORKSHOP,   5, 5, "Wrk" }
};

void buildings_init()
{
	models[BUILDING_LABORATORY].color = TCOD_yellow;
	models[BUILDING_WORKSHOP].color = TCOD_green;
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

building* building_create(int model, int x, int y)
{
	building *b = malloc(sizeof(building));
	b->model = &models[model];
	b->x = x;
	b->y = y;
	b->next = buildings;
	return b;
}

void building_add(int model, int x, int y)
{
	building *b = building_create(model, x, y);
	b->next = buildings;
	buildings = b;
}

void building_draw(building *b)
{
	TCOD_color_t old_color = TCOD_console_get_foreground_color(NULL);
	TCOD_console_set_foreground_color(NULL, b->model->color);
	TCOD_console_print_frame(NULL, b->x, b->y,
		b->model->width, b->model->height,
		false, TCOD_BKGND_NONE, NULL);
	TCOD_console_print_left(NULL, b->x + 1, b->y, TCOD_BKGND_NONE, b->model->code);
	TCOD_console_set_foreground_color(NULL, old_color);
}

void buildings_draw()
{
	for (building *b = buildings; b != NULL; b = b->next)
		building_draw(b);
}
