#include "proceduralDunGen.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>


int main()
{
    srand(time(0));
    int rows = 50, cols = 100;
    int **map;

    map = malloc(rows * sizeof *map);
    for (int i=0; i < rows; i++)
        map[i] = malloc(cols * sizeof *map[i]);

    generate(map, rows, cols);

    for(int i = 0; i < rows; i++)
    {
        for(int j = 0; j < cols; j++)
            if(map[i][j] == 0)
                printf("%d", map[i][j]);
            else
                printf(" ");
        puts("");
    }

}
