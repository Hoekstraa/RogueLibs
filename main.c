#include <SDL2/SDL.h>
#include "lib/borrowed/queue.h"
#include "lib/proceduralDunGen.h"
#include "lib/directionFlooder.h"
#include "lib/datatypes.h"
#include "lib/vision.h"
#include "data/values.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

SDL_Window *window = 0; // Global window
SDL_Renderer *renderer = 0; // Global renderer
SDL_Texture *mapTexture = 0; // Global background

//const int rows = MAP_ROWS, cols = MAP_COLS; // Size of map
int **map; // Global map

SDL_Texture *textures[10];
SDL_Texture *walltextures[3];
enum texture {PLAYERTEX, ENEMYTEX};

Entity *player;

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
    window = SDL_CreateWindow(WINDOW_NAME, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN );
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

    mapTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, MAP_COLS*TILESIZE, MAP_ROWS*TILESIZE);
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
    map = malloc(MAP_ROWS * sizeof *map);
    for (int i=0; i < MAP_ROWS; i++)
        map[i] = malloc(MAP_COLS * sizeof *map[i]);

    // Create dungeon map.
    generate(map, MAP_ROWS, MAP_COLS);

    const SDL_Rect bmpRect = {0,0,TILESIZE,TILESIZE};
    SDL_Rect destRect = {0,0,TILESIZE,TILESIZE};

    //Temporarily render to texture
    SDL_SetRenderTarget(renderer, mapTexture);

    // Print dungeon map.
    for(int i = 0; i < MAP_ROWS; i++)
        for(int j = 0; j < MAP_COLS; j++)
            if(map[i][j] == WALL) {
                destRect.x = TILESIZE*j;
                destRect.y = TILESIZE*i;
                SDL_RenderCopy(renderer, walltextures[rand()%3], &bmpRect, &destRect);
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
    // Make wall texture
    SDL_Surface *bmps4 = SDL_LoadBMP("img/wall2.bmp");
    SDL_Texture *bmpt4 = SDL_CreateTextureFromSurface(renderer, bmps4);
    SDL_FreeSurface(bmps4);
    // Make wall texture
    SDL_Surface *bmps5 = SDL_LoadBMP("img/wall3.bmp");
    SDL_Texture *bmpt5 = SDL_CreateTextureFromSurface(renderer, bmps5);
    SDL_FreeSurface(bmps5);
    walltextures[0] = bmpt;
    walltextures[1] = bmpt4;
    walltextures[2] = bmpt5;

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

// Return a coord where an entity (incl. player) could spawn
coord sprinkle ()
{
    coord c;
    while(1)
    {
        c.x = rand() % MAP_COLS;
        c.y = rand() % MAP_ROWS;

        if(freeCoord(c)) return c;
    }
}

void spawnPlayer()
{
    player = malloc(sizeof(Entity));
    player->damage = PLAYER_DAMAGE;
    player->health = PLAYER_HEALTH;
    player->player = 1;
    player->texture = PLAYERTEX;
    player->c = sprinkle();
}

void sprinkleEntities(int amount)
{
    for(int i = 0; i < amount; i++)
    {
        e1 = malloc(sizeof(struct listitem));	/* Insert at the head. */
        e1->entity.texture = ENEMYTEX;
        e1->entity.damage = SNAIL_DAMAGE;
        e1->entity.health = SNAIL_HEALTH;
        e1->entity.player = 0;
        e1->entity.c = sprinkle();
        LIST_INSERT_HEAD(&entities, e1, listitems);
    }
}
//-------------------------------------------------------------------------------------
// From here on out all functions are ran in the gameloop, so performance is important!
//-------------------------------------------------------------------------------------

void kill(Entity *entity)
{
    if (entity->player)
    {
        puts("You have died!");
        SDL_Quit();
        exit(0);
    }
    else
        for(ep2 = LIST_FIRST(&entities); ep2 != NULL; ep2 = LIST_NEXT(ep2, listitems))
            if(ep2->entity.c.x == entity->c.x && ep2->entity.c.y == entity->c.y)
            {
                LIST_REMOVE(ep2, listitems);
                free(ep2);
                break;
            }
}

void attack(coord c, Entity *entity)
{
    if(map[c.y][c.x] != FLOOR) return;

    if(c.x == player->c.x && c.y == player->c.y)
    {
        player->health -= entity->damage;
        if (player->health <= 0) kill(player);
    }

    LIST_FOREACH(ep2, &entities, listitems)
    if(ep2->entity.c.x == c.x && ep2->entity.c.y == c.y)
    {
        ep2->entity.health -= entity->damage;
        if (ep2->entity.health <= 0) kill(&(ep2->entity));
    }
}


void moveOrAttack(coord c, Entity *entity)
{
    if(freeCoord(c))
    {
        entity->c.x = c.x;
        entity->c.y = c.y;
    }
    else attack(c, entity);
}

void moveEntities()
{
    int **distanceMap;
    distanceMap = malloc(MAP_ROWS * sizeof *distanceMap);
    for (int i=0; i < MAP_ROWS; i++)
        distanceMap[i] = malloc(MAP_COLS * sizeof *distanceMap[i]);

    // Zero memory just in case
    for (int i=0; i < MAP_ROWS; i++)
        for(int j=0; j < MAP_COLS; j++)
            distanceMap[i][j] = 0;

    // Generate direction map.
    flood(map, distanceMap, player->c.y, player->c.x);

    // Go towards the player.
    LIST_FOREACH(ep, &entities, listitems)
    {
        if (canSee(map, ep->entity.c, player->c))
        {
            coord c;
            c.x = ep->entity.c.x;
            c.y = ep->entity.c.y;
            int direction = distanceMap[c.y][c.x];

            switch(direction) {
            case NORTH:
                --c.y;
                break;
            case SOUTH:
                ++c.y;
                break;
            case WEST:
                --c.x;
                break;
            case EAST:
                ++c.x;
                break;
            }
            moveOrAttack(c, &ep->entity);
        }
    }
    for(int i = 0; i < MAP_ROWS; i++)
        free(distanceMap[i]);
    free(distanceMap);
}

void movePlayer(int direction)
{
    coord c;
    c.x = player->c.x;
    c.y = player->c.y;
    switch(direction) {
    case NORTH:
        --c.y;
        break;
    case SOUTH:
        ++c.y;
        break;
    case WEST:
        --c.x;
        break;
    case EAST:
        ++c.x;
        break;
    }

    moveOrAttack(c, player);
    // When the player has moved, the other entities get a turn.
    moveEntities();
}

void handleKey(SDL_Keycode key)
{
    if (key == SDLK_UP || key == SDLK_k) movePlayer(NORTH);
    if (key == SDLK_DOWN || key == SDLK_j) movePlayer(SOUTH);
    if (key == SDLK_LEFT || key == SDLK_h) movePlayer(WEST);
    if (key == SDLK_RIGHT || key == SDLK_l) movePlayer(EAST);
    if (key == SDLK_PERIOD) moveEntities();
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
            if(event.key.keysym.sym == SDLK_q)
                // Break out of the gameloop on quit
                return 0;
            else
                handleKey(event.key.keysym.sym);
        }
    }
    return 1;
}

int min(int a, int b) {
    return a < b? a:b;
}
int max(int a, int b) {
    return a > b? a:b;
}

void render()
{
    SDL_RenderClear(renderer);

    SDL_Texture *tempTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, MAP_COLS*TILESIZE, MAP_ROWS*TILESIZE);
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
        const SDL_Rect mapRect = {0,0, MAP_COLS*TILESIZE,MAP_ROWS*TILESIZE};
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
        if (camX > (MAP_COLS*TILESIZE - CAMERA_WIDTH*TILESIZE)) camX = MAP_COLS*TILESIZE - CAMERA_WIDTH*TILESIZE;
        int camY = player->c.y*TILESIZE - ((CAMERA_HEIGHT*TILESIZE) / 2);
        if (camY < 0) camY = 0;
        if(camY > (MAP_ROWS*TILESIZE - CAMERA_HEIGHT*TILESIZE)) camY = MAP_ROWS*TILESIZE - CAMERA_HEIGHT*TILESIZE;


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
    srand(time(0)); // Initialize (seed) randomizer with current time.
    initSDL(); // Create screen, etc.
    loadTextures(); // Load in textures to the GPU.
    createDungeon(); // Generate random map and create a texture of it for rendering.
    spawnPlayer(); // Spawn player in random location in the map.
    LIST_INIT(&entities); //Initialize the (global) list of enemies.
    sprinkleEntities(ENEMY_AMOUNT); // Spawn (x amount of) enemies in random locations in the map.

    while (handleEvents()) // As long as there's no quit event, handle other events and do..
    {
        render();
    }

    //SDL_DestroyTexture(mapTexture);
    //SDL_DestroyRenderer(renderer);
    //SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
