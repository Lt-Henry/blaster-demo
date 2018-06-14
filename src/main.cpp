
#include <blaster/raster.h>
#include <blaster/vector.h>
#include <blaster/vbo.h>

#include <SDL2/SDL.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>

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

bl_vbo_t* build_points_vbo(Mesh* mesh)
{
    bl_vbo_t* vbo;
    
    struct point_t {
        bl_vector_t p;
        bl_color_t c;
    };
    
    vbo=bl_vbo_new(mesh->vertices.size(),sizeof(point_t));
    
    for (int n=0;n<mesh->vertices.size();n++) {
        struct point_t point = {0};
        
        point.p=mesh->vertices[n];
        
        bl_vbo_set(vbo,n,&point);
    }
    
    return vbo;
}

bl_vbo_t* build_lines_vbo(Mesh* mesh)
{
    bl_vbo_t* vbo;
    
    struct point_t {
        bl_vector_t p;
        bl_color_t c;
    };
    
    vbo=bl_vbo_new(mesh->triangles.size()*6,sizeof(point_t));
    
    int m=0;
    for (int n=0;n<mesh->triangles.size();n++) {
        point_t point = {0};
        
        point.p=mesh->vertices[mesh->triangles[n].v[0]];
        bl_vbo_set(vbo,m,&point);
        
        point.p=mesh->vertices[mesh->triangles[n].v[1]];
        bl_vbo_set(vbo,m+1,&point);
        
        point.p=mesh->vertices[mesh->triangles[n].v[1]];
        bl_vbo_set(vbo,m+2,&point);
        
        point.p=mesh->vertices[mesh->triangles[n].v[2]];
        bl_vbo_set(vbo,m+3,&point);
        
        point.p=mesh->vertices[mesh->triangles[n].v[2]];
        bl_vbo_set(vbo,m+4,&point);
        
        point.p=mesh->vertices[mesh->triangles[n].v[0]];
        bl_vbo_set(vbo,m+5,&point);
        
        m+=6;
    }
    
    return vbo;
}

int main(int argc,char* argv[])
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    
    bl_raster_t* raster;
    bl_vbo_t* dino;
    
    clog<<"Blaster-demo"<<endl;
    
    Mesh* mesh = load_obj(argv[1]);
    
    clog<<"vertices: "<<mesh->vertices.size()<<endl;
    clog<<"triangles: "<<mesh->triangles.size()<<endl;
    
    raster=bl_raster_new(WIDTH,HEIGHT);
    
    bl_color_t clear_color;
    bl_color_set(&clear_color,0.9,0.9,0.9,1.0);
    
    bl_raster_set_clear_color(raster,&clear_color);
    bl_raster_clear(raster);
    
    dino=build_lines_vbo(mesh);
    
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow("blaster", 100, 100, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH,HEIGHT);
    
    
    auto tfps = std::chrono::steady_clock::now();
    
    double dfps=0;
    int fps=0;
    double time_clear=0;
    double time_raster=0;
    double time_upload=0;
    double time_present=0;
    
    float angle=0;
    float aspeed=0.1f;
    
    float Z=-30;
    
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
            
            case SDL_MOUSEWHEEL:
                if (event.wheel.y>0) {
                    Z+=10.0f;
                }
                
                if (event.wheel.y<0) {
                    Z+=-10.0f;
                }
                
            break;

            } // switch
        } // while
        
        
        //render here
        auto t1 = std::chrono::steady_clock::now();
        
        bl_raster_clear(raster);
        
        auto t2 = std::chrono::steady_clock::now();
        time_clear+=std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count();
        
        
        bl_matrix_stack_load_identity(raster->projection);
        bl_matrix_stack_frustum(raster->projection,
        -1,1,-1,1,1,1000);
        
        bl_matrix_stack_load_identity(raster->modelview);
        bl_matrix_stack_translate(raster->modelview,0.0f,0.0f,Z);

        angle+=0.005f;
        bl_matrix_stack_rotate_y(raster->modelview,angle);
        
        bl_raster_draw_lines(raster,dino);
        bl_raster_update(raster);
        
        SDL_Rect rect;

        rect.x=0;
        rect.y=0;
        rect.w=WIDTH;
        rect.h=HEIGHT;

        auto t3 = std::chrono::steady_clock::now();
        
        SDL_UpdateTexture(texture,&rect,(void*)raster->color_buffer->data,WIDTH*sizeof(uint32_t));
        
        auto t4 = std::chrono::steady_clock::now();
        time_upload+=std::chrono::duration_cast<std::chrono::milliseconds>(t4-t3).count();
        
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        
        auto t5 = std::chrono::steady_clock::now();
        time_present+=std::chrono::duration_cast<std::chrono::milliseconds>(t5-t4).count();
        
        fps++;
        dfps=std::chrono::duration_cast<std::chrono::milliseconds>(t5-tfps).count();
        
        if (dfps>1000) {
            clog<<"fps: "<<fps<<endl;
            clog<<"clear: "<<time_clear<<" ms"<<endl;
            clog<<"upload: "<<time_upload<<" ms"<<endl;
            clog<<"present: "<<time_present<<" ms"<<endl;

            aspeed=((2.0f*3.14f)*fps)/1000.0f;

            dfps=0;
            fps=0;
            time_clear=0;
            time_upload=0;
            time_present=0;
            
            tfps = std::chrono::steady_clock::now();
        }
    }
    
    bl_raster_delete(raster);
    
    return 0;
}
