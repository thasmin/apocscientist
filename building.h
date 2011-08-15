#ifndef BUILDING_H
#define BUILDING_H

#include <tcod/libtcod.h>
#include "point.h"

#define BUILDING_LABORATORY	0
#define BUILDING_WORKSHOP	1
#define BUILDING_STORAGE	2

typedef struct _building_model {
	int model;
	int width, height;
	const char *code;
	TCOD_color_t color;
} building_model;

typedef struct _building {
	building_model *model;
	point p;
	struct _building *next;
} building;

void buildings_init();
building_model* building_get_model(int model);

void building_add(int model, point *p);
building* building_create(int model, point *p);
void building_draw(building *b);
void buildings_draw();

building* building_find_closest(point *p, int model);
void building_adjust_to_center(building *b, point *p);

#endif
