#include <SDL2/SDL.h>
#include "lib/proceduralDunGen.h"
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
enum texture{WALLTEX = 0, PLAYERTEX=1};

struct Player
{
    int x;
    int y;
} *player;

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
    //SDL_RenderSetIntegerScale(renderer, 1);
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
}

void spawnPlayer()
{
    player = malloc(sizeof(struct Player));
    int spawnCol;
    int spawnRow;
    while(1)
    {
        spawnRow = rand() % rows;
        spawnCol = rand() % cols;
        if(map[spawnRow][spawnCol] == FLOOR)
        {
            player->y = spawnRow;
            player->x = spawnCol;
            break;
        }
    }
}

void moveEntities()
{

}

void movePlayer(int direction)
{
    if (direction == NORTH)
        if(map[player->y - 1][player->x] != WALL)
            player->y = player->y - 1;

    if (direction == SOUTH)
        if(map[player->y + 1][player->x] != WALL)
            player->y = player->y + 1;

    if (direction == WEST)
        if(map[player->y][player->x - 1] != WALL)
            player->x = player->x - 1;

    if (direction == EAST)
        if(map[player->y][player->x + 1] != WALL)
            player->x = player->x + 1;

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
    const SDL_Rect mapRect = {0,0, cols*TILESIZE,rows*TILESIZE};
    SDL_RenderCopy(renderer, mapTexture,&mapRect,&mapRect);

    // Render player
    const SDL_Rect bmpRect = {0,0,TILESIZE,TILESIZE};
    const SDL_Rect destRect2 = {(player->x)*TILESIZE,(player->y)*TILESIZE,TILESIZE,TILESIZE};
    SDL_RenderCopy(renderer, textures[PLAYERTEX], &bmpRect, &destRect2);


    // Future render calls render to renderer itself again.
    SDL_SetRenderTarget(renderer, 0);
    //---------------------------------------------------------------------------------------------------
    
    // Determine camera location (incl. clamping boundaries)

    int camX = player->x*TILESIZE - ((CAMERA_WIDTH*TILESIZE) / 2);
    if(camX < 0) camX = 0;
    if (camX > (cols*TILESIZE - CAMERA_WIDTH*TILESIZE)) camX = cols*TILESIZE - CAMERA_WIDTH*TILESIZE;
    int camY = player->y*TILESIZE - ((CAMERA_HEIGHT*TILESIZE) / 2);
    if (camY < 0) camY = 0;
    if(camY > (rows*TILESIZE - CAMERA_HEIGHT*TILESIZE)) camY = rows*TILESIZE - CAMERA_HEIGHT*TILESIZE;

    
    const SDL_Rect cameraRect = {camX, camY, CAMERA_WIDTH*TILESIZE, CAMERA_HEIGHT*TILESIZE};
    const SDL_Rect screenRect = {0,0,CAMERA_WIDTH*TILESIZE,CAMERA_HEIGHT*TILESIZE};

    SDL_RenderCopy(renderer, tempTexture, &cameraRect, &screenRect);

    // Present to screen
    SDL_RenderPresent(renderer);

    free(tempTexture);
}

int main(int argc, char ** argv)
{
    srand(time(0));
    initSDL();
    loadTextures();
    createDungeon();
    spawnPlayer();

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
