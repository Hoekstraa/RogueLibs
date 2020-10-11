#include "directionFlooder.h"

int main()
{
    int rows = 20, cols = 40;
    int **map, **distanceMap;

    // Create fully dynamic 2D array
    map = malloc(rows * sizeof *map);
    for (int i=0; i < rows; i++)
        map[i] = malloc(cols * sizeof *map[i]);
    
    // Fills array with wall tiles
    for (int i = 0; i < rows; i++)
        for(int j = 0; j < cols; j++)
            map[i][j] = WALL;

    // Make an interesting map to test the flooding with
    for(int i = 5; i < (rows - 3); i++)
        for(int j = 4; j < (cols - 7); j++)
            map[i][j] = FLOOR;
    
    for (int i = 6; i < 10; i++)
        map[i][12] = WALL;

    // Make a distanceMap for the library to fill in.
    distanceMap = malloc(rows * sizeof *map);
    for (int i=0; i < rows; i++)
        distanceMap[i] = malloc(cols * sizeof *map[i]);

    flood(map, distanceMap,7,7);

    //print distanceMap
    for(int i = 0; i < rows; i++)
    {
        for(int j = 0; j < cols; j++)
            if(distanceMap[i][j] == 0)
                printf("█");
            else
                printf("%d", distanceMap[i][j]);
        printf("\n");
    }

}
