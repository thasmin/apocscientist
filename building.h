#ifndef BUILDING_H
#define BUILDING_H

#include <tcod/libtcod.h>
#include "point.h"

#define BUILDING_LABORATORY	0
#define BUILDING_WORKSHOP	1

typedef struct _building_model {
	int model;
	int width, height;
	const char *code;
	TCOD_color_t color;
} building_model;

typedef struct _building {
	building_model *model;
	int x, y;
	struct _building *next;
} building;

void buildings_init();
void building_add(int model, point *p);
building* building_create(int model, point *p);
void building_draw(building *b);
void buildings_draw();

#endif
