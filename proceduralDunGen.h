#include<stdlib.h>
#include<stdio.h>

//Number of objects to generate on the map
//#define DunGen_object

enum tile {WALL, FLOOR} tile;

void fillRectangle(int **map, int row, int col, int width, int height, enum tile floortype)
{
    for(int i = row; i < row+width; i++)
        for(int j = col; j < col+height; j++)
            map[i][j] = floortype; 
}

enum direction {NORTH = 1, EAST = 2, SOUTH = 3, WEST = 4} direction;

int isGoodWall(int **map, const int mapwidth, const int mapheight, const int row, const int col)
{
    if(map[row][col] != WALL) return 0;
    // prevent underflow, check one up for open tile
    if(row > 0 && map[row-1][col] == FLOOR) return NORTH;
    //prevent underflow, check one tile to the left
    if(col > 0 && map[row][col-1] == FLOOR) return WEST;
    //prevent overflow, check one tile down
    if(row < mapwidth - 1 && map[row+1][col] == FLOOR) return SOUTH;
    //prevent overflow, check one tile right
    if(col < mapwidth - 1 && map[row][col+1] == FLOOR) return EAST;

    puts("Not good!");
    return 0;
}

struct coord{int col; int row;} coord;

struct coord pickWall(int **map, const int rows, const int cols)
{
   struct coord c; 
    while(1)
    {
        c.row = rand() % rows;
        c.col = rand() % cols;
        if(isGoodWall(map, rows, cols, c.row, c.col))
            return c;
    }
}

void buildFeature(int **map, int rows, int cols)
{
    struct coord c = pickWall(map, rows, cols);
    printf("Good location is [%d][%d]", c.row,c.col);
}



/// Takes a 2d int array and generates a random dungeon in it.
void generate(int **map, const int rows, const int cols)
{
    puts("Cleaning array..");
    //Cleans array, in case a dirty array is put in.
    for (int i = 0; i < rows; i++)
        for(int j = 0; j < cols; j++){
            printf("Clearing [%d][%d]", i,j);
            map[i][j] = 0;
        }
    puts("cleaned array");

    //start with a random square somewhat in the middle
    puts("Making beginning room..");
    int minimum = 2;
    fillRectangle(map, rows/2, cols/2, rand()%15 + minimum, rand()%10 + minimum, FLOOR);
    puts("Made beginning room");
    buildFeature(map, rows,cols);
}
