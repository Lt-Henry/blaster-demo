
#include <blaster/raster.h>
#include <blaster/vector.h>

#include <SDL2/SDL.h>

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

#define WIDTH 800
#define HEIGHT 600

struct Triangle
{
    int v[3];
    int n[3];
};

struct Mesh
{
    vector<bl_vector_t> vertices;
    vector<bl_vector_t> normals;
    vector<Triangle> triangles;
};

vector<string> split(string line,char sep=' ')
{
    vector<string> tokens;
    
    bool knee=false;
    string tmp;

    for (char c : line) {

        if (c==sep) {
            if (knee==true) {
                tokens.push_back(tmp);
                knee=false;
                tmp="";
            }
        }
        else {
            tmp=tmp+c;
            knee=true;
        }
    }

    if (tmp!="") {
        tokens.push_back(tmp);
    }
    
    return tokens;
}

Mesh* load_obj(char* filename)
{
    clog<<"loading: "<<filename<<endl;
    
    fstream fs;
    
    setlocale(LC_NUMERIC,"C");
    
    fs.open(filename,fstream::in);
    
    Mesh* mesh = new Mesh();
    
    while (!fs.eof()) {
        string line;
        getline(fs,line);
        
        vector<string> tmp = split(line);
        
        if (tmp.size()<=0) {
            continue;
        }
        
        if (tmp[0]=="v") {
            bl_vector_t v;
            
            v.x=stof(tmp[1]);
            v.y=stof(tmp[2]);
            v.z=stof(tmp[3]);
            v.w=1;
            
            mesh->vertices.push_back(v);
        }
        
        if (tmp[0]=="f") {
            Triangle tri;
            
            vector<string> tmp2 = split(tmp[1],'/');
            tri.v[0]=stoi(tmp2[0]);
            
            tmp2 = split(tmp[2],'/');
            tri.v[1]=stoi(tmp2[0]);
            
            tmp2 = split(tmp[3],'/');
            tri.v[2]=stoi(tmp2[0]);
            
            mesh->triangles.push_back(tri);
        }
    }
    
    fs.close();
    
    return mesh;
}


int main(int argc,char* argv[])
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    
    clog<<"Blaster-demo"<<endl;
    
    Mesh* mesh = load_obj(argv[1]);
    
    clog<<"vertices: "<<mesh->vertices.size()<<endl;
    clog<<"triangles: "<<mesh->triangles.size()<<endl;
    
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
