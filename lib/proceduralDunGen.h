#include<stdlib.h>
#include<stdio.h>

#define MINPATHLENGTH 2
#define MAXPATHLENGTH 10
#define MINROOMSIZE 4
#define MAXROOMSIZE 12
#define AMOUNTOFROOMS 36

enum tile {WALL = -1, FLOOR = -2} tile;
enum direction {NORTH = 1, EAST = 2, SOUTH = 3, WEST = 4} direction;
struct inertPoint{int col; int row; int dir;} inertPoint;

/// Naively fill a rectangle of height & width at map[row][col] with floortype.
void fillRectangle(int **map, int row, int col, int height, int width, enum tile floortype)
{
    for(int i = row; i < row+height; i++)
        for(int j = col; j < col+width; j++)
            map[i][j] = floortype;
}

/// See if [row][col] is a wall next to a room. Return the direction a path can be created.
int isGoodWall(int **map, const int mapheight, const int mapwidth, const int row, const int col)
{
    if(row < 1 || row > (mapheight - 2) || col < 1 || col > (mapwidth -2) || map[row][col] != WALL) return 0;

    // Check surrounding tiles for open tile (which means a room is there, so go the other way).
    if(map[row-1][col] == FLOOR) return SOUTH;
    if(map[row][col-1] == FLOOR) return EAST;
    if(map[row+1][col] == FLOOR) return NORTH;
    if(map[row][col+1] == FLOOR) return WEST;

    return 0;
}

/// Look for a suitable tile to start a path from.
/// Uses randomization.
/// Returns row, column and direction with an inertPoint.
struct inertPoint pickWall(int **map, const int rows, const int cols)
{
    struct inertPoint c;
    while(1)
    {
        c.row = rand() % rows;
        c.col = rand() % cols;
        c.dir = isGoodWall(map, rows, cols, c.row, c.col);
        if(c.dir) return c; // isGoodWall will return 0 if it doesn't find a good wall, so this will return only if this is a valid direction.
    }
}

/// Step 'tiles' amount of tiles in the direction of inert.
struct inertPoint follow(struct inertPoint inert, const int tiles)
{
    if (inert.dir == NORTH) inert.row = inert.row - tiles;
    if (inert.dir == SOUTH) inert.row = inert.row + tiles;
    if (inert.dir == WEST) inert.col = inert.col - tiles;
    if (inert.dir == EAST) inert.col = inert.col + tiles;

    return inert;
}

/// Test if a hypothetical path doesn't go out of bounds of the map array.
int testPath(int **map, int rows, int cols, struct inertPoint inert, int length)
{
    if (length < 1) return 0;
    if (inert.dir == NORTH)
        while(length != 0){
            --inert.row; --length;
            if(inert.row <= 1) return 0;
            if(map[inert.row][inert.col] != WALL) return 0;

        }
    if (inert.dir == SOUTH)
        while(length != 0){
            ++inert.row; --length;
            if(inert.row >= (rows - 2)) return 0;
            if(map[inert.row][inert.col] != WALL) return 0;
        }
    if (inert.dir == WEST)
        while(length != 0){
            --inert.col; --length;
            if(inert.col <= 1) return 0;
            if(map[inert.row][inert.col] != WALL) return 0;
        }
    if (inert.dir == EAST)
        while(length != 0){
            ++inert.col; --length;
            if(inert.col >= (cols - 2)) return 0;
            if(map[inert.row][inert.col] != WALL) return 0;
        }
    return 1;
}

///Replace WALL with FLOOR for 'length' tiles in direction inert.dir at location inert(row,col)
void buildPath(int **map, struct inertPoint inert, int length)
{
    if(length < 1) return;

    if (inert.dir == NORTH)
        while(length-- != 0)
            map[inert.row - length][inert.col] = FLOOR;
    if (inert.dir == SOUTH)
        while(length-- != 0)
            map[inert.row + length][inert.col] = FLOOR;
    if (inert.dir == WEST)
        while(length-- != 0)
            map[inert.row][inert.col - length] = FLOOR;
    if (inert.dir == EAST)
        while(length-- != 0)
            map[inert.row][inert.col + length] = FLOOR;
}

/// See if the block and surrounding blocks are available.
//
// Used to make sure a room can be placed.
// Has a lot of checks to make sure it doesn't go out of bounds of the array.
int testSurrounding(int **map, int rows, int cols, int row, int col)
{
    if(row < 1 || col < 1 || row > (rows - 2) || col > (cols - 2) || map[row][col] != WALL) return 0; // Test that the block itself is a wall and thus available

    if // Test that blocks surrounding [row][col] are available
        (
         map[row-1][col] != WALL ||
         map[row+1][col] != WALL ||
         map[row][col-1] != WALL ||
         map[row][col+1] != WALL ||
         map[row-1][col-1] != WALL ||
         map[row-1][col+1] != WALL ||
         map[row+1][col-1] != WALL ||
         map[row+1][col+1] != WALL
        ) return 0;

    return 1; // Block and all surrounding blocks are wall and thus safe to build on.
}

/// See if an area in the map is available for a room to be placed.
/// A little helper function looping testSurrounding().
int testArea(int **map, int rows, int cols, int row, int col, int height, int width)
{
    for(int i = 0; i < height; i++)
        for(int j = 0; j < width; j++)
        {
            int available = testSurrounding(map, rows, cols, row+i, col+j);
            if(!available) return 0;
        }
    return 1;
}

/// Try and build a room at inert.
/// Uses random values generated in the function for size etc.
int buildRoom(int **map, int rows, int cols, struct inertPoint inert)
{
    //printf("Testing a room at [%d][%d] towards %d\n", inert.row, inert.col,inert.dir);
    int height = (rand() % (MAXROOMSIZE - MINROOMSIZE)) + MINROOMSIZE;
    int width = (rand() % (MAXROOMSIZE - MINROOMSIZE)) + MINROOMSIZE;

    if(inert.dir == NORTH)
    {
        int offset = rand() % (width - 1); // To move the room a bit so the path isn't always in a corner

        if(testArea(map, rows, cols, inert.row - height + 1, inert.col - offset, height, width))
        {
            fillRectangle(map, inert.row - height + 1, inert.col - offset, height, width, FLOOR);
            return 1;
        }
    }

    if(inert.dir == SOUTH)
    {
        int offset = rand() % (width - 1); // To move the room a bit so the path isn't always in a corner

        if(testArea(map, rows, cols, inert.row, inert.col - offset, height, width))
        {
            fillRectangle(map, inert.row, inert.col - offset, height, width, FLOOR);
            return 1;
        }
    }

    if(inert.dir == WEST)
    {
        int offset = rand() % (height - 1); // To move the room a bit so the path isn't always in a corner

        if(testArea(map, rows, cols, inert.row - offset, inert.col - width + 1, height, width))
        {
            fillRectangle(map, inert.row - offset, inert.col - width + 1, height, width, FLOOR);
            return 1;
        }
    }

    if(inert.dir == EAST)
    {
        int offset = rand() % (height - 1); // To move the room a bit so the path isn't always in a corner

        if(testArea(map, rows, cols, inert.row - offset, inert.col, height, width))
        {
            fillRectangle(map, inert.row - offset, inert.col, height, width, FLOOR);
            return 1;
        }
    }

    return 0;
}

/// Find a spot to start an offshoot of an existing room, to place a path and room there.
///
/// 1. Randomly searches for a suitable wall that sits next to a room.
/// 2. When found, it tests to see if a path (random length) could be made there
/// 3. If this is the case, try and build a room there
/// 4. If the room could be made, retroactively attach the path to it.
void buildFeature(int **map, int rows, int cols)
{
    struct inertPoint c;
    int pathLength;
    struct inertPoint startOfRoom;

    do {
        do {
            c = pickWall(map, rows,cols);
            pathLength = (rand() % (MAXPATHLENGTH - MINPATHLENGTH)) + MINPATHLENGTH;

        } while(!testPath(map, rows, cols, c, pathLength)); // Keep testing random paths until a suitable is found.

        startOfRoom = follow(c, pathLength);
    } while(!buildRoom(map, rows, cols, startOfRoom)); // Keep trying out room configurations until a suitable is found.

    buildPath(map, c, pathLength);
}

/// Takes a 2d int array and generates a random dungeon in it.
void generate(int **map, const int rows, const int cols)
{
    // Fills array with wall tiles
    for (int i = 0; i < rows; i++)
        for(int j = 0; j < cols; j++)
            map[i][j] = WALL;

    //start with a random rectangle somewhat in the middle
    fillRectangle(map, rows/2, cols/2, rand()%12 + MINROOMSIZE, rand()%10 + MINROOMSIZE, FLOOR);

    // Generate rooms around it
    for(int i = 0; i < AMOUNTOFROOMS; i++) buildFeature(map, rows, cols);
}
