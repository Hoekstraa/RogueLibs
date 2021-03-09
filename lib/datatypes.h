#ifndef DATATYPES_H
#define DATATYPES_H

const enum tile {WALL = -1, FLOOR = -2} tile;
const enum direction {NORTH = 1, EAST = 2, SOUTH = 3, WEST = 4, DESTINATION = 5} direction;
typedef struct coord{int y; int x;} coord;

typedef struct Entity
{
    coord c;
    int damage;
    int health;
    int texture;
    int player;
} Entity;

#endif
