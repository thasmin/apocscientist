#include <stdio.h>
#include <string.h>
#include <math.h>

#include "map.h"
#include "task.h"

point_node *items[ITEM_COUNT];
int map[MAP_COLS * MAP_ROWS];
unsigned storage[ITEM_COUNT];

TCOD_map_t tcod_map;

void map_init()
{
	memset(items, 0, sizeof(point_node*) * ITEM_COUNT);
	memset(storage, 0, sizeof(unsigned) * ITEM_COUNT);
	tcod_map = TCOD_map_new(MAP_COLS, MAP_ROWS);
	TCOD_map_clear(tcod_map, true, true);
}

void map_destroy()
{
	for (int i = 0; i < ITEM_COUNT; ++i)
	{
		point_node *node = items[i];
		while (node != NULL) {
			point_node *next = node->next;
			free(node);
			node = next;
		}
	}

	TCOD_map_delete(tcod_map);
}

inline int mapindex(int x, int y)
{
	return x + y * MAP_COLS;
}

inline int point_mapindex(point *p)
{
	return mapindex(p->x, p->y);
}

int map_item_at(int x, int y)
{
	return map[mapindex(x, y)];
}

point_node *map_item_list(int item)
{
	return items[item];
}

int map_create_item(int x, int y, int item)
{
	map[mapindex(x, y)] = item;
	point_node *node;
	node = malloc(sizeof(point_node));
	node->p.x = x;
	node->p.y = y;
	node->next = items[item];
	items[item] = node;
	return 1;
}

// returns whether an item was picked up
int map_pickup_item(point *p)
{
	int item = map[point_mapindex(p)];
	map[point_mapindex(p)] = ITEM_NONE;

	// remove from the item list
	point_node *picked_up = items[item];
	point_node *prev = NULL;
	while (picked_up != NULL) {
		if (point_equals(&picked_up->p, p))
			break;
		prev = picked_up;
		picked_up = picked_up->next;
	}
	if (picked_up == NULL)
		return 0;
	if (prev)
		prev->next = picked_up->next;
	else
		items[item] = items[item]->next;
	free(picked_up);

	return item;
}

// returns whether an item was dropped
int map_drop_item(point *p, int item)
{
	if (item == ITEM_NONE)
		return 0;
	return map_create_item(p->x, p->y, item);
}

void map_set_walkable(int x, int y, bool is_walkable)
{
	// fourth argument is is_transparent
	TCOD_map_set_properties(tcod_map, x, y, true, is_walkable);
}

TCOD_path_t map_computepath(point *origin, point *dest)
{
	//printf("origin %f, %f\n", origin->x, origin->y);
	//printf("dest %f, %f\n", dest->x, dest->y);
	// diagonal cost is sqrt(2)
	TCOD_path_t path = TCOD_path_new_using_map(tcod_map, 1.41f);
	TCOD_path_compute(path, origin->x, origin->y, dest->x, dest->y);

	/*
	bool is_possible = TCOD_path_compute(path, origin->x, origin->y, dest->x, dest->y);
	printf("is_possible: %d\n", is_possible);
	printf("path steps: %d\n", TCOD_path_size(path));
	for (int i = 0; i < TCOD_path_size(path); ++i) {
		int x, y;
		TCOD_path_get(path, i, &x, &y);
		printf("step %d: %d, %d\n", i + 1, x, y);
	}
	*/
	return path;
}

void map_walk(TCOD_path_t path, point *p, float dist)
{
	int x, y;
	TCOD_path_get_origin(path, &x, &y);
	//printf("origin: %d, %d\n", x, y);
	//printf("curr  : %f, %f\n", p->x, p->y);
	TCOD_path_get(path, 0, &x, &y);
	//printf("step 0: %d, %d\n", x, y);

	// if the next step is closer than dist, move to it
	// then subtract how far was moved from dist and repeat
	float mx = x - p->x;
	float my = y - p->y;
	//printf("m     : %f, %f\ndist : %f\n", mx, my, dist);
	while (mx * mx + my * my < dist * dist) {
		p->x = x;
		p->y = y;
		dist -= sqrt(mx * mx + my * my);

		TCOD_path_walk(path, &x, &y, true);
		//printf("next step: %d, %d\n", x, y);
		mx = x - p->x;
		my = y - p->y;
		if (mx == 0.0f && my == 0.0f)
			return;
		//printf("m     : %f, %f\ndist : %f\n", mx, my, dist);
	}

	float pctm = dist / sqrt(mx * mx + my * my);
	p->x += pctm * mx;
	p->y += pctm * my;
	//printf("dist  : %f\n", dist);
	//printf("final : %f, %f\n", p->x, p->y);
	//printf("\n");
}

point* map_find_closest(point *p, int item)
{
	if (items[item] == NULL)
		return NULL;

	// find the closest bucket
	point *closest_bucket = &items[item]->p;
	float closest_bucket_dist = point_dist(p, closest_bucket);
	point_node *pn = items[item]->next;
	while (pn) {
		float dist = point_dist(p, &pn->p);
		if (dist < closest_bucket_dist) {
			closest_bucket_dist = dist;
			closest_bucket = &pn->p;
		}
		pn = pn->next;
	}
	return closest_bucket;
}

void storage_add(int item)
{
	++storage[item];
}

int storage_take(int item)
{
	if (storage[item] <= 0)
		return 0;
	storage[item]--;
	return 1;
}

unsigned storage_get_count(int item)
{
	return storage[item];
}
