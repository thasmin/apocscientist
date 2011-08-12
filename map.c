#include <stdio.h>

#include "map.h"

/*
point_node *buckets;
point_node *wells;
point_node **items[] = { 0, &buckets, &wells };
*/
point_node *items[] = { NULL, NULL, NULL };
int map[MAP_COLS * MAP_ROWS];

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
