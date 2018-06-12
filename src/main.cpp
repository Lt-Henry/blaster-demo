
#include <blaster/raster.h>

#include <SDL2/SDL.h>

#include <iostream>

using namespace std;

#define WIDTH 800
#define HEIGHT 600

int main(int argc,char* argv[])
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    
    clog<<"Blaster-demo"<<endl;
    
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow("blaster", 100, 100, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH,HEIGHT);
    
    bool quit_request=false;
    
    while(!quit_request) {
        SDL_Event event;
        
        // eat events
        while(SDL_PollEvent(&event)) {

            switch (event.type) {
            case SDL_QUIT:
                clog<<"quit request"<<endl;
                quit_request=true;
            break;

            } // switch
        } // while
        
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
    
    return 0;
}
