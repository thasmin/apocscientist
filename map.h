#ifndef MAP_H
#define MAP_H

#include "point.h"

#define MAP_COLS	80
#define MAP_ROWS	50

#define ITEM_NONE 	0
#define ITEM_BUCKET 	1
#define ITEM_WELL	2
#define ITEM_SCREW	3
#define ITEM_COUNT	4

int mapindex(int x, int y);
int point_mapindex(point *p);

int map_item_at(int x, int y);
point_node *map_item_list(int item);

point* map_find_closest(point *p, int item);
int map_create_item(int x, int y, int item);
int map_pickup_item(point *p, int item);
int map_drop_item(point *p, int item);

#endif
