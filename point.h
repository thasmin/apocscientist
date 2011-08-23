#ifndef POINT_C
#define POINT_C

#include <stdlib.h>

typedef struct _point {
	float x;
	float y;
} point;

typedef struct _point_node {
	point p;
	struct _point_node *next;
} point_node;

void point_invalidate(point *p);
int point_is_valid(point *p);

float point_dist(point *p1, point *p2);
int point_equals(point *p1, point *p2);
void point_moveto(point *mover, point *target, float dist);

void debug_print_point(point *p);
void debug_print_point_node_list(point_node *node);

#endif
