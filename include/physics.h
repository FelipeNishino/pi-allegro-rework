#ifndef PHYSICS_H
#define PHYSICS_H

typedef enum {
	group_player = 1,
	group_enemy = 1 << 1,
	group_utility_1 = 1 << 2,
	group_utility_2 = 1 << 3,
	group_utility_3 = 1 << 4,
	group_utility_4 = 1 << 5,
	group_utility_5 = 1 << 6,
	group_utility_6 = 1 << 7,
} entity_group_mask;

typedef struct point {
	int x;
	int y;
} Point;

typedef struct fpoint {
	float x;
	float y;
} FPoint;

typedef struct size {
	int width;
	int height;
} Size;

typedef struct rectangle {
	Point origin;
	Size size;
} Rect;

typedef struct hitbox {
	FPoint anchor;
	Rect rect;
} Hitbox;

#endif //PHYSICS_H