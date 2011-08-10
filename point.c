#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "point.h"

#ifndef PI
#define PI 3.14159265
#endif

// actually dist^2
float point_dist(point *p1, point *p2)
{
	if (p1 == p2)
		return 0;
	if (p1 == NULL || p2 == NULL)
		return 0;
	return pow((int)p1->x - (int)p2->x, 2) + pow((int)p1->y - (int)p2->y, 2);
}

int point_equals(point *p1, point *p2)
{
	if (p1 == p2)
		return 1;
	if (p1 == NULL || p2 == NULL)
		return 0;
	return (int)p1->x == (int)p2->x && (int)p1->y == (int)p2->y;
}

void point_moveto(point *mover, point *target, float dist)
{
	if (point_equals(mover, target))
		return;
	float movex = (int)target->x - (int)mover->x;
	float movey = (int)target->y - (int)mover->y;
	float angle = atan2(movey, movex) * 180 / PI;
	if (angle < 0)
		angle += 360;
	// 0 is right, 90 is down
	float step = 360.0f / 16;
	if (angle > 360 - step || angle < 0 + step)
		mover->x += dist;
	else if (angle > 45 - step && angle < 45 + step)
		mover->x += dist, mover->y += dist;
	else if (angle > 90 - step && angle < 90 + step)
		mover->y += dist;
	else if (angle > 135 - step && angle < 135 + step)
		mover->x -= dist, mover->y += dist;
	else if (angle > 180 - step && angle < 180 + step)
		mover->x -= dist;
	else if (angle > 225 - step && angle < 225 + step)
		mover->x -= dist, mover->y -= dist;
	else if (angle > 270 - step && angle < 270 + step)
		mover->y -= dist;
	else if (angle > 315 - step && angle < 315 + step)
		mover->x += dist, mover->y -= dist;
	else
		printf("bad angle: %f\n", angle);
}

void debug_print_point(point *p)
{
	printf("%f, %f\n", p->x, p->y);
}

void debug_print_point_node_list(point_node *node)
{
	int i = 0;
	while (node) {
		printf("%d: %f, %f\n", i, node->p.x, node->p.y);
		++i;
		node = node->next;
	}
}
