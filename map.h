#ifndef MAP_H
#define MAP_H

#include "point.h"

#include <tcod/libtcod.h>

#define MAP_COLS	80
#define MAP_ROWS	50

void map_init();
void map_destroy();

int mapindex(int x, int y);
int point_mapindex(point *p);

int map_item_at(int x, int y);
point_node *map_item_list(int item);

void storage_add(int item);
int storage_take(int item);
unsigned storage_get_count(int item);

void map_set_walkable(int x, int y, bool is_walkable);
TCOD_path_t map_computepath(point *origin, point *dest);
void map_walk(TCOD_path_t path, point *p, float dist);

point* map_find_closest(point *p, int item);
int map_create_item(int x, int y, int item);
// returns what item was picked up
int map_pickup_item(point *p);
int map_drop_item(point *p, int item);

#endif
