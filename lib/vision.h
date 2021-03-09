#include <stdlib.h>
#include <stdio.h>
#include "datatypes.h"

/// Test if two coords can 'see' eachother
/// How: Bresenham's line algorithm. If a point in that line is wall, then no, else yes.
int canSeeInt(int **map, int x0, int y0, int x1, int y1)
{
    const int dx = abs(x1-x0);
    const int sx = x0<x1 ? 1 : -1;
    const int dy = -abs(y1-y0);
    const int sy = y0<y1 ? 1 : -1;
    int err = dx + dy; // error value e_xy
    int e2;

    for(;;)
    {
        if (map[y0][x0] == WALL) return 0; // coords cannot see eachother
        if (x0 == x1 && y0 == y1) return 1; // coords can see eachother

        e2 = 2*err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

/// Like canSeeInt, but with coords
int canSee(int **map, coord a, coord b)
{
    return canSeeInt(map, a.x, a.y, b.x, b.y);
}
