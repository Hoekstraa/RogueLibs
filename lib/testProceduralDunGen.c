#include "proceduralDunGen.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main()
{
    srand(time(0));
    int rows = 50, cols = 100;
    int **map;

    // Create a 2D array of fully dynamic size
    map = malloc(rows * sizeof *map);
    for (int i=0; i < rows; i++)
        map[i] = malloc(cols * sizeof *map[i]);

    // Create dungeon map.
    generate(map, rows, cols);

    // Print dungeon map.
    for(int i = 0; i < rows; i++)
    {
        for(int j = 0; j < cols; j++)
            if(map[i][j] == WALL)
                printf("█");
            else
                printf(" ");
        printf("\n");
    }

}
