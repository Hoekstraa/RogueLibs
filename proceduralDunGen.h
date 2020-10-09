#include<stdlib.h>
#include<stdio.h>

//Number of objects to generate on the map
//#define DunGen_object

#define MINPATHLENGTH 2
#define MAXPATHLENGTH 10
#define MINROOMSIZE 4
#define MAXROOMSIZE 12

enum tile {WALL, FLOOR} tile;

void fillRectangle(int **map, int row, int col, int height, int width, enum tile floortype)
{
    for(int i = row; i < row+height; i++)
        for(int j = col; j < col+width; j++)
            map[i][j] = floortype; 
}

enum direction {NORTH = 1, EAST = 2, SOUTH = 3, WEST = 4} direction;

int isGoodWall(int **map, const int mapheight, const int mapwidth, const int row, const int col)
{
    if(row < 1 || row > (mapheight - 2) || col < 1 || col > (mapwidth -2) || map[row][col] != WALL) return 0;

    // prevent underflow, check one up for open tile (which means a room is there, so go the other way)
    if(map[row-1][col] == FLOOR) return SOUTH;
    //prevent underflow, check one tile to the left
    if(map[row][col-1] == FLOOR) return EAST;
    //prevent overflow, check one tile down
    if(map[row+1][col] == FLOOR) return NORTH;
    //prevent overflow, check one tile right
    if(map[row][col+1] == FLOOR) return WEST;

    return 0;
}

struct inertPoint{int col; int row; int dir;} inertPoint;

struct inertPoint pickWall(int **map, const int rows, const int cols)
{
    struct inertPoint c; 
    while(1)
    {
        c.row = rand() % rows;
        c.col = rand() % cols;
        c.dir = isGoodWall(map, rows, cols, c.row, c.col);
        if(c.dir != 0) printf("testing direction %d", c.dir);
        if(c.dir) return c; // isGoodWall will return 0 if it doesn't find a good will, so this is a tiny hack
    }
}

/// Step 'tiles' amount of tiles in the direction of inert
struct inertPoint follow(struct inertPoint inert, const int tiles)
{
    if (inert.dir == NORTH) inert.row = inert.row - tiles;
    if (inert.dir == SOUTH) inert.row = inert.row + tiles;
    if (inert.dir == WEST) inert.col = inert.col - tiles;
    if (inert.dir == EAST) inert.col = inert.col + tiles;

    return inert;
}
/// Test if hypothetical path doesn't go out of bounds of array
int testPath(int **map, int rows, int cols, struct inertPoint inert, int length)
{
    puts("Testing path");
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
    puts("path test failed");
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
// Had a lot of checks to make sure it doesn't go out of bounds of the array.
int testSurrounding(int **map, int rows, int cols, int row, int col)
{
    if(row < 1 || col < 1 || row > (rows - 2) || col > (cols - 2) || map[row][col] != WALL) return 0; // Test that the block itself is a wall and thus available

    if(row > 0 && map[row-1][col] != WALL) return 0;// Test that block up is available
    if(row < rows-1 && map[row+1][col] != WALL) return 0;// Test that block below is available

    if(col > 0 && map[row][col-1] != WALL) return 0;// Test that block left is available
    if(col < col-1 && map[row][col+1] != WALL) return 0;// Test that block right is available

    if(row > 0  && col > 0 && map[row-1][col-1] != WALL) return 0;// Test that block left/up is available
    if(row > 0  && col < col-1 && map[row-1][col+1] != WALL) return 0;// Test that block right/up is available
    if(row < row-1  && col > 0 && map[row+1][col-1] != WALL) return 0;// Test that block left/down is available
    if(row < row-1  && col < col-1 && map[row+1][col+1] != WALL) return 0;// Test that block right/down is available
    puts("Surrounding is safe.");
    return 1; // Block and all surrounding blocks are wall and thus safe to build on.
}

/// See if an area in the map is available for a room to be placed
int testArea(int **map, int rows, int cols, int row, int col, int height, int width)
{
    for(int i = 0; i < height; i++)
        for(int j = 0; j < width; j++)
        {
            printf("Testing [%d][%d]\n", row+i, col+j);
            int available = testSurrounding(map, rows, cols, row+i, col+j);
            if(!available) return 0;
        }
    puts("area is suitable");
    return 1;
}

int buildRoom(int **map, int rows, int cols, struct inertPoint inert)
{
    //printf("Testing a room at [%d][%d] towards %d\n", inert.row, inert.col,inert.dir);
    int height = (rand() % (MAXROOMSIZE - MINROOMSIZE)) + MINROOMSIZE;
    int width = (rand() % (MAXROOMSIZE - MINROOMSIZE)) + MINROOMSIZE;

    if(inert.dir == NORTH)
    {
        int offset = rand() % (width - 1); // To move the room a bit so the path isn't always in a corner

        //printf("%d, %d, %d, %d, %d\n", inert.row, inert.col + offset, height, width, inert.row - height + 1);
        if(testArea(map, rows, cols, inert.row - height + 1, inert.col - offset, height, width))
        {
            puts("Area is safe to build in!");
            fillRectangle(map, inert.row - height + 1, inert.col - offset, height, width, FLOOR);
            puts("Scraped out room!");
            return 1;
        }
    }

    if(inert.dir == SOUTH)
    {
        int offset = rand() % (width - 1); // To move the room a bit so the path isn't always in a corner

        //printf("%d, %d, %d, %d\n", inert.row, inert.col - offset, height, width);
        if(testArea(map, rows, cols, inert.row, inert.col - offset, height, width))
        {
            puts("Area is safe to build in!");
            fillRectangle(map, inert.row, inert.col - offset, height, width, FLOOR);
            puts("Scraped out room!");
            return 1;
        }
    }

    if(inert.dir == WEST)
    {
        int offset = rand() % (height - 1); // To move the room a bit so the path isn't always in a corner

        //printf("%d, %d, %d, %d, %d\n", inert.row + offset, inert.col - width + 1, height, width, inert.row);
        if(testArea(map, rows, cols, inert.row - offset, inert.col - width + 1, height, width))
        {
            puts("Area is safe to build in!");
            fillRectangle(map, inert.row - offset, inert.col - width + 1, height, width, FLOOR);
            puts("Scraped out room!");
            return 1;
        }
    }

    if(inert.dir == EAST)
    {
        int offset = rand() % (height - 1); // To move the room a bit so the path isn't always in a corner

        //printf("%d, %d, %d, %d\n", inert.row - offset, inert.col, height, width);
        if(testArea(map, rows, cols, inert.row - offset, inert.col, height, width))
        {
            puts("Area is safe to build in!");
            fillRectangle(map, inert.row - offset, inert.col, height, width, FLOOR);
            puts("Scraped out room!");
            return 1;
        }
    }

    return 0;
}

void buildFeature(int **map, int rows, int cols)
{
    struct inertPoint c;
    int pathLength;
    struct inertPoint startOfRoom;

    do {
        do {
            c = pickWall(map, rows,cols);
            pathLength = (rand() % (MAXPATHLENGTH - MINPATHLENGTH)) + MINPATHLENGTH;

        } while(!testPath(map, rows, cols, c, pathLength));

        printf("Good path of length %d\n", pathLength);
        startOfRoom = follow(c, pathLength);
    } while(!buildRoom(map, rows, cols, startOfRoom));
    puts("Room built!");
    buildPath(map, c, pathLength);
}

/// Takes a 2d int array and generates a random dungeon in it.
void generate(int **map, const int rows, const int cols)
{
    puts("Cleaning array..");
    //Cleans array, in case a dirty array is put in.
    for (int i = 0; i < rows; i++)
        for(int j = 0; j < cols; j++){
            //printf("Clearing [%d][%d]", i,j);
            map[i][j] = 0;
        }
    puts("cleaned array");

    //start with a random rectangle somewhat in the middle
    puts("Making beginning room..");
    fillRectangle(map, rows/2, cols/2, rand()%12 + MINROOMSIZE, rand()%10 + MINROOMSIZE, FLOOR);
    puts("Made beginning room");

    for(int i = 0; i < 40; i++) buildFeature(map, rows, cols);
}
