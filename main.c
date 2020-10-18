#include <SDL2/SDL.h>
#include "lib/borrowed/queue.h"
#include "lib/proceduralDunGen.h"
#include "lib/directionFlooder.h"
#include "lib/datatypes.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900
#define CAMERA_WIDTH 32 //Amount of tiles
#define CAMERA_HEIGHT 18 //Amount of tiles
#define TILESIZE 16
SDL_Window *window = 0; // Global window
SDL_Renderer *renderer = 0; // Global renderer
SDL_Texture *mapTexture = 0; // Global background

const int rows = 50, cols = 100; // Size of map
int **map; // Global map

SDL_Texture *textures[10];
enum texture{WALLTEX = 0, PLAYERTEX=1, ENEMYTEX=2};

struct Player
{
    coord c;
} *player;

///Enemy related////////////////////////
typedef struct Entity
{
    coord c;
    int damage;
    int health;
    int texture;
} Entity;

LIST_HEAD(listhead, listitem) entities;
struct listitem {
    Entity entity;
    LIST_ENTRY(listitem) listitems;
} *e1, *e2, *ep, *ep2;
///////////////////////////////////////

void initSDL()
{
    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        exit(1);
    }
    //Create window
    window = SDL_CreateWindow("Rogey", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN );
    if( window == 0 )
    {
        printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        SDL_Quit();
        exit(2);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if( renderer == 0 )
    {
        printf( "Renderer could not be created! SDL_Error: %s\n", SDL_GetError() );
        SDL_Quit();
        exit(2);
    }
    SDL_RenderSetIntegerScale(renderer, 1);
    SDL_RenderSetLogicalSize(renderer, CAMERA_WIDTH*TILESIZE,CAMERA_HEIGHT*TILESIZE);

    mapTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, cols*TILESIZE, rows*TILESIZE);
    if( mapTexture == 0 )
    {
        printf( "map texture could not be created! SDL_Error: %s\n", SDL_GetError() );
        SDL_Quit();
        exit(2);
    }
}

void createDungeon()
{
    // Create a 2D array of fully dynamic size
    //free(map);
    map = malloc(rows * sizeof *map);
    for (int i=0; i < rows; i++)
        map[i] = malloc(cols * sizeof *map[i]);

    // Create dungeon map.
    generate(map, rows, cols);

    const SDL_Rect bmpRect = {0,0,TILESIZE,TILESIZE};
    SDL_Rect destRect = {0,0,TILESIZE,TILESIZE};

    //Temporarily render to texture
    SDL_SetRenderTarget(renderer, mapTexture);

    // Print dungeon map.
    for(int i = 0; i < rows; i++)
        for(int j = 0; j < cols; j++)
            if(map[i][j] == WALL){
                destRect.x = TILESIZE*j;
                destRect.y = TILESIZE*i;
                SDL_RenderCopy(renderer, textures[WALLTEX], &bmpRect, &destRect);
            }
    // Future render calls render to renderer itself again.
    SDL_SetRenderTarget(renderer, 0);
}

void loadTextures()
{
    // Make wall texture
    SDL_Surface *bmps = SDL_LoadBMP("img/wall.bmp");
    SDL_Texture *bmpt = SDL_CreateTextureFromSurface(renderer, bmps);
    SDL_FreeSurface(bmps);
    textures[WALLTEX] = bmpt;

    //Make player texture
    SDL_Surface *bmps2 = SDL_LoadBMP("img/player.bmp");
    SDL_Texture *bmpt2 = SDL_CreateTextureFromSurface(renderer, bmps2);
    SDL_FreeSurface(bmps2);
    textures[PLAYERTEX] = bmpt2;

    //Make snail texture
    SDL_Surface *bmps3 = SDL_LoadBMP("img/snail.bmp");
    SDL_Texture *bmpt3 = SDL_CreateTextureFromSurface(renderer, bmps3);
    SDL_FreeSurface(bmps3);
    textures[ENEMYTEX] = bmpt3;
}

// Tests wether a coordinate is free to spawn/walk on or not.
int freeCoord(coord c)
{
    if(map[c.y][c.x] != FLOOR) return 0;
    if(player->c.x == c.x && player->c.y == c.y) return 0;
    
    LIST_FOREACH(ep2, &entities, listitems)
        if(ep2->entity.c.x == c.x && ep2->entity.c.y == c.y) return 0;
    
    return 1;
}

int freeCoordXY(int x, int y)
{
    coord a;
    a.x = x;
    a.y = y;
    return freeCoord(a);
}

// Return a coord where an entity (incl. player) could spawn
coord sprinkle ()
{
    coord c;
    while(1)
    {
        c.x = rand() % cols;
        c.y = rand() % rows;

        if(freeCoord(c)) return c;
    }
}

void spawnPlayer()
{
    player = malloc(sizeof(struct Player));
    player->c = sprinkle();
}

void sprinkleEntities()
{
    for(int i = 0; i < 5; i++)
    {
        e1 = malloc(sizeof(struct listitem));	/* Insert at the head. */
        e1->entity.texture = ENEMYTEX;
        e1->entity.c = sprinkle();
        LIST_INSERT_HEAD(&entities, e1, listitems);
    }
}
//-------------------------------------------------------------------------------------
// From here on out all functions are ran in the gameloop, so performance is important!
//-------------------------------------------------------------------------------------
void moveEntities()
{
    int **distanceMap;
    distanceMap = malloc(rows * sizeof *distanceMap);
    for (int i=0; i < rows; i++)
        distanceMap[i] = malloc(cols * sizeof *distanceMap[i]);

    // Generate direction map.
    flood(map, distanceMap, player->c.y, player->c.x);

    // Go towards the player.
    LIST_FOREACH(ep, &entities, listitems)
    {
        int direction = distanceMap[ep->entity.c.y][ep->entity.c.x];
        coord c;
        c.x = ep->entity.c.x;
        c.y = ep->entity.c.y;
        switch(direction){
            case NORTH: --c.y; break;
            case SOUTH: ++c.y; break;
            case WEST: --c.x; break;
            case EAST: ++c.x; break;
        }
        if(freeCoord(c))
        {
            ep->entity.c.x = c.x;
            ep->entity.c.y = c.y;
        }
    }
    free(distanceMap);
}

void movePlayer(int direction)
{
    if (direction == NORTH)
        if(freeCoordXY(player->c.x, player->c.y - 1))
            player->c.y = player->c.y - 1;

    if (direction == SOUTH)
        if(freeCoordXY(player->c.x, player->c.y + 1))
            player->c.y = player->c.y + 1;

    if (direction == WEST)
        if(freeCoordXY(player->c.x - 1, player->c.y))
            player->c.x = player->c.x - 1;

    if (direction == EAST)
        if(freeCoordXY(player->c.x + 1, player->c.y))
            player->c.x = player->c.x + 1;

    // When the player has moved, the other entities get a turn.
    moveEntities();
}

void handleKey(SDL_Keycode key)
{
    if (key == SDLK_UP) movePlayer(NORTH);
    if (key == SDLK_DOWN) movePlayer(SOUTH);
    if (key == SDLK_LEFT) movePlayer(WEST);
    if (key == SDLK_RIGHT) movePlayer(EAST);
}

int handleEvents()
{
    // Get the next event
    SDL_Event event;
    if (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
            // Break out of the gameloop on quit
            return 0;
        if (event.type == SDL_KEYDOWN)
        {
            // Brean out of the gameloop on quit
            if(event.key.keysym.sym == SDLK_q)
                return 0;
            else
                handleKey(event.key.keysym.sym);
        }
    }
    return 1;
}

int min(int a, int b){return a < b? a:b;}
int max(int a, int b){return a > b? a:b;}

void render()
{
    SDL_RenderClear(renderer);

    SDL_Texture *tempTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, cols*TILESIZE, rows*TILESIZE);
    if( tempTexture == 0 )
    {
        printf( "Temporary texture could not be created! SDL_Error: %s\n", SDL_GetError() );
        SDL_Quit();
        exit(2);
    }

    //Temporarily render to texture
    SDL_SetRenderTarget(renderer, tempTexture);
    //---------------------------------------------------------------------------------------------------

    //Copy map to temp texture.
    {
        const SDL_Rect mapRect = {0,0, cols*TILESIZE,rows*TILESIZE};
        SDL_RenderCopy(renderer, mapTexture,&mapRect,&mapRect);
    }
    // Render player
    {
        const SDL_Rect bmpRect = {0,0,TILESIZE,TILESIZE};
        const SDL_Rect destRect = {(player->c.x)*TILESIZE,(player->c.y)*TILESIZE,TILESIZE,TILESIZE};
        SDL_RenderCopy(renderer, textures[PLAYERTEX], &bmpRect, &destRect);
    }
    // Render entities
    {
        const SDL_Rect bmpRect = {0,0,TILESIZE,TILESIZE};
        LIST_FOREACH(ep, &entities, listitems)
        {
            coord c = ep->entity.c;
            const SDL_Rect destRect = {c.x*TILESIZE, c.y*TILESIZE,TILESIZE,TILESIZE};
            SDL_RenderCopy(renderer, textures[ep->entity.texture], &bmpRect, &destRect);
        }
    }

    // Future render calls render to renderer itself again.
    SDL_SetRenderTarget(renderer, 0);
    //---------------------------------------------------------------------------------------------------

    // Render part of temp texture, to scroll with the player as a camera.
    {
        // Determine camera location (incl. clamping boundaries)
        int camX = player->c.x*TILESIZE - ((CAMERA_WIDTH*TILESIZE) / 2);
        if(camX < 0) camX = 0;
        if (camX > (cols*TILESIZE - CAMERA_WIDTH*TILESIZE)) camX = cols*TILESIZE - CAMERA_WIDTH*TILESIZE;
        int camY = player->c.y*TILESIZE - ((CAMERA_HEIGHT*TILESIZE) / 2);
        if (camY < 0) camY = 0;
        if(camY > (rows*TILESIZE - CAMERA_HEIGHT*TILESIZE)) camY = rows*TILESIZE - CAMERA_HEIGHT*TILESIZE;


        const SDL_Rect cameraRect = {camX, camY, CAMERA_WIDTH*TILESIZE, CAMERA_HEIGHT*TILESIZE};
        const SDL_Rect screenRect = {0,0,CAMERA_WIDTH*TILESIZE,CAMERA_HEIGHT*TILESIZE};
        SDL_RenderCopy(renderer, tempTexture, &cameraRect, &screenRect);
    }
    // Present to screen
    SDL_RenderPresent(renderer);

    SDL_DestroyTexture(tempTexture);
}

int main(int argc, char ** argv)
{
    srand(time(0));
    initSDL();
    loadTextures();
    createDungeon();
    spawnPlayer();
    LIST_INIT(&entities);/* Initialize list. */
    sprinkleEntities();

    while (handleEvents())
    {
        render();
    }

    //SDL_DestroyTexture(mapTexture);
    //SDL_DestroyRenderer(renderer);
    //SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
