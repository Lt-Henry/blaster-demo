
#include <blaster/raster.h>
#include <blaster/vector.h>
#include <blaster/vbo.h>
#include <blaster/time.h>
#include <blaster/tga.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <tiny_gltf.h>
#include <SDL2/SDL.h>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <list>
#include <memory>

using namespace std;

#define WIDTH 1920
#define HEIGHT 1080

enum class RenderMode {
    Points,
    Lines,
    Triangles
};

struct Triangle
{
    int v[3];
    int n[3];
    int t[3];
};

struct Mesh
{
    vector<bl_vector_t> vertices;
    vector<bl_vector_t> normals;
    vector<bl_uv_t> uvs;
    
    vector<Triangle> triangles;
};

std::ostream& operator<<(std::ostream& os,Triangle& t)
{
    os<<"("<<t.v[0]<<" "<<t.v[1]<<" "<<t.v[2]<<") ("<<t.n[0]<<" "<<t.n[1]<<" "<<t.n[2]<<")";
    
    return os;
}

vector<string> split(string line,char sep=' ')
{
    vector<string> tokens;
    
    bool knee=false;
    string tmp;

    for (char c : line) {

        if (c==sep) {
            tokens.push_back(tmp);
            tmp="";
        }
        else {
            tmp=tmp+c;
        }
    }

    tokens.push_back(tmp);
    
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
        
        if (tmp[0]=="vn") {
            bl_vector_t n;
            
            n.x=stof(tmp[1]);
            n.y=stof(tmp[2]);
            n.z=stof(tmp[3]);
            n.w=0;
            
            bl_vector_normalize(&n);
            mesh->normals.push_back(n);
        }
        
        if (tmp[0]=="vt") {
            bl_uv_t uv;
            
            uv.u=stof(tmp[1]);
            uv.v=stof(tmp[2]);
            
            mesh->uvs.push_back(uv);
        }
        
        if (tmp[0]=="f") {
            Triangle tri;
            
            vector<string> tmp2 = split(tmp[1],'/');
            //clog<<">>"<<tmp2[0]<<","<<tmp2[1]<<endl;
            tri.v[0]=stoi(tmp2[0])-1;
            tri.t[0]=stoi(tmp2[1])-1;
            tri.n[0]=stoi(tmp2[2])-1;
            
            tmp2 = split(tmp[2],'/');
            tri.v[1]=stoi(tmp2[0])-1;
            tri.t[1]=stoi(tmp2[1])-1;
            tri.n[1]=stoi(tmp2[2])-1;
            
            tmp2 = split(tmp[3],'/');
            tri.v[2]=stoi(tmp2[0])-1;
            tri.t[2]=stoi(tmp2[1])-1;
            tri.n[2]=stoi(tmp2[2])-1;
            
            mesh->triangles.push_back(tri);
            /*
            clog<<mesh->normals[tri.n[0]].x<<","<<mesh->normals[tri.n[0]].y<<","<<mesh->normals[tri.n[0]].z<<endl;
            clog<<mesh->normals[tri.n[1]].x<<","<<mesh->normals[tri.n[1]].y<<","<<mesh->normals[tri.n[1]].z<<endl;
            clog<<mesh->normals[tri.n[2]].x<<","<<mesh->normals[tri.n[2]].y<<","<<mesh->normals[tri.n[2]].z<<endl;
            */
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
    
    vbo=bl_vbo_new(mesh->vertices.size(),8);
    
    for (size_t n=0;n<mesh->vertices.size();n++) {
        struct point_t point = {0};
        
        point.p=mesh->vertices[n];
        
        bl_vbo_set_v(vbo,n,&point);
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
    
    vbo=bl_vbo_new(mesh->triangles.size()*6,8);
    
    int m=0;
    for (size_t n=0;n<mesh->triangles.size();n++) {
        point_t point = {0};
        
        point.p=mesh->vertices[mesh->triangles[n].v[0]];
        bl_vbo_set_v(vbo,m,&point);
        
        point.p=mesh->vertices[mesh->triangles[n].v[1]];
        bl_vbo_set_v(vbo,m+1,&point);
        
        point.p=mesh->vertices[mesh->triangles[n].v[1]];
        bl_vbo_set_v(vbo,m+2,&point);
        
        point.p=mesh->vertices[mesh->triangles[n].v[2]];
        bl_vbo_set_v(vbo,m+3,&point);
        
        point.p=mesh->vertices[mesh->triangles[n].v[2]];
        bl_vbo_set_v(vbo,m+4,&point);
        
        point.p=mesh->vertices[mesh->triangles[n].v[0]];
        bl_vbo_set_v(vbo,m+5,&point);
        
        m+=6;
    }
    
    return vbo;
}

bl_vbo_t* build_triangles_vbo(Mesh* mesh)
{
    bl_vbo_t* vbo;
    
    struct point_t {
        bl_vector_t p;
        bl_vector_t n;
        bl_uv_t t;
    };
    
    vbo=bl_vbo_new(mesh->triangles.size()*3,10); //4,4,2
    
    int m=0;
    for (size_t n=0;n<mesh->triangles.size();n++) {
        point_t point = {0};
        
        point.p=mesh->vertices[mesh->triangles[n].v[0]];
        point.n=mesh->normals[mesh->triangles[n].n[0]];
        point.t=mesh->uvs[mesh->triangles[n].t[0]];
        bl_vbo_set_v(vbo,m,&point);
        
        point.p=mesh->vertices[mesh->triangles[n].v[1]];
        point.n=mesh->normals[mesh->triangles[n].n[1]];
        point.t=mesh->uvs[mesh->triangles[n].t[1]];
        bl_vbo_set_v(vbo,m+1,&point);

        point.p=mesh->vertices[mesh->triangles[n].v[2]];
        point.n=mesh->normals[mesh->triangles[n].n[2]];
        point.t=mesh->uvs[mesh->triangles[n].t[2]];
        bl_vbo_set_v(vbo,m+2,&point);
        
        m+=3;
    }
    
    return vbo;
}

bool load_gltf(tinygltf::Model &model, const char *filename) {
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
    if (!warn.empty()) {
    std::clog << "WARN: " << warn << std::endl;
    }

    if (!err.empty()) {
    std::clog << "ERR: " << err << std::endl;
    }

    if (!res)
    std::clog << "Failed to load glTF: " << filename << std::endl;
    else
    std::clog << "Loaded glTF: " << filename << std::endl;

    return res;
}

bl_vbo_t* build_vbo(tinygltf::Model &model)
{
    bl_vbo_t* vbo = nullptr;

    for (tinygltf::BufferView& bv : model.bufferViews) {
        clog<<"buffer "<<bv.buffer<<endl;
        clog<<"\toffset "<<bv.byteOffset<<endl;
        clog<<"\tsize "<<bv.byteLength<<endl;
        clog<<"\ttarget "<<bv.target<<endl;
    }

    for (tinygltf::Accessor& ac : model.accessors) {
        //clog<<"accesor "<<ac.name<<endl;
    }

    for (tinygltf::Mesh& mesh : model.meshes) {
        for (tinygltf::Primitive& primitive : mesh.primitives) {
            for (auto k : primitive.attributes) {
                clog<<"* "<<k.first<<":"<<k.second<<endl;
            }
        }
    }

    return vbo;
}

void print_time(string name,double value,int fps)
{
    double f=1.0/1000.0;
    double ratio = value/10000.0;
    clog<<"* "<<name<<": "<<value*f<<" ms ["<<value*f/fps<<"] "<<ratio<<"%"<<endl;
}

int main(int argc,char* argv[])
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_GLContext gl;
    
    bl_raster_t* raster;
    bl_vbo_t* vbo;
    
    RenderMode mode = RenderMode::Triangles;
    //RenderMode mode = RenderMode::Lines;
    
    clog<<"Blaster-demo"<<endl;


    if (argc<2) {
        cerr<<"Missing GLTF file"<<endl;
        return -1;
    }

    tinygltf::Model model;

    if (!load_gltf(model,argv[1])) {
        cerr<<"Failed to load gltf file"<<endl;
        return -1;
    }

    clog<<"meshes: "<<model.meshes.size()<<endl;

    vbo = build_vbo(model);

    return 0;
    
    Mesh* mesh = load_obj(argv[1]);
    
    clog<<"vertices: "<<mesh->vertices.size()<<endl;
    clog<<"triangles: "<<mesh->triangles.size()<<endl;

    raster=bl_raster_new(WIDTH,HEIGHT,3,1);
    
    if (argc>2) {
        bl_texture_t* tx = bl_tga_load(argv[2]);
        bl_raster_set_texture(raster,tx);
    }

    switch (mode) {
        case RenderMode::Points:
            vbo=build_points_vbo(mesh);
        break;
        
        case RenderMode::Lines:
            vbo=build_lines_vbo(mesh);
        break;
        
        case RenderMode::Triangles:
            vbo=build_triangles_vbo(mesh);
        break;
    
    }


    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow("blaster", 100, 100, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH,HEIGHT);

    bl_color_t clear_color;
    bl_color_set(&clear_color,0.9,0.9,0.9,1.0);

    bl_raster_set_clear_color(raster,&clear_color);

    auto tfps = std::chrono::steady_clock::now();
    
    double dfps=0;
    int fps=0;
    int min_fps=100000;
    int max_fps=0;
    list<int> avg_fps;
    
    double time_input=0;
    double time_clear=0;
    double time_raster_draw=0;
    double time_raster_update=0;
    double time_upload=0;
    double time_present=0;
    double time_total=0;
    
    float angle=0;
    float aspeed=0.01f;
    
    float Z=-30;
    float Y=0;
    
    bool quit_request=false;
    
    bl_pixel_t pick;
    
    bool request_data=false;
    int rx,ry;
    
    while(!quit_request) {
        SDL_Event event;
        
        auto t0a = std::chrono::steady_clock::now();
        // eat events
        while(SDL_PollEvent(&event)) {

            switch (event.type) {
                case SDL_QUIT:
                    clog<<"quit request"<<endl;
                    quit_request=true;
                break;
                
                case SDL_MOUSEWHEEL:
                    if (event.wheel.y<0) {
                        Z=Z*1.25f;
                    }
                    
                    if (event.wheel.y>0) {
                        Z=Z/2.0f;
                    }
                    
                break;
                
                case SDL_MOUSEBUTTONDOWN:
                    request_data=true;
                    rx = event.button.x;
                    ry = event.button.y;
                    
                break;
                
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym==SDLK_UP) {
                        Y-=1;
                    }
                    if (event.key.keysym.sym==SDLK_DOWN) {
                        Y+=1;
                    }
                break;
            
            } // switch
        } // while
        
        auto t0b = std::chrono::steady_clock::now();
        time_input+=std::chrono::duration_cast<std::chrono::microseconds>(t0b-t0a).count();
        
        //render here
        auto t1 = std::chrono::steady_clock::now();
        
        raster->start=bl_time_us();
        bl_raster_clear(raster);

        auto t2 = std::chrono::steady_clock::now();
        time_clear+=std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count();
        
        auto t2a = std::chrono::steady_clock::now();
        
        float aspect=WIDTH/(float)HEIGHT;
        
        /*
        bl_matrix_stack_load_identity(raster->projection);
        bl_matrix_stack_frustum(raster->projection,
        -aspect,aspect,-1,1,1,100);
        
        bl_matrix_stack_load_identity(raster->modelview);
        bl_matrix_stack_translate(raster->modelview,0.0f,Y,Z);

        angle+=0.0025f;
        bl_matrix_stack_rotate_y(raster->modelview,angle);

        bl_matrix_t mvp; //modelview * projection matrix
        bl_matrix_mult(&mvp,raster->projection->matrix,raster->modelview->matrix);

        bl_raster_uniform_set_matrix(raster,0 , &mvp);
        */

        glm::mat4 mprojection = glm::frustum(-aspect,aspect,1.0f,-1.0f,1.0f,100.0f);

        angle+=0.0025f;
        glm::mat4 mmodel(1.0f);
        mmodel = glm::translate(mmodel,glm::vec3(0.0f,Y,Z));
        mmodel = glm::rotate(mmodel,angle,glm::vec3(0.0f,1.0f,0.0f));

        glm::mat4 mvp = mprojection * mmodel;

        bl_raster_uniform_set_matrix(raster,0 , (bl_matrix_t*)&mvp[0][0]);
        bl_raster_uniform_set_matrix(raster,1 , (bl_matrix_t*)&mmodel[0][0]);
        //bl_raster_uniform_set_matrix(raster,1 , raster->modelview->matrix);

        bl_vector_t light_pos = {0.0f,1.0f,4.0f,0.0f};
        bl_vector_normalize(&light_pos);
        bl_raster_uniform_set_vector(raster,2,&light_pos);
        
        auto t2b = std::chrono::steady_clock::now();

        switch (mode) {
            case RenderMode::Points:
                bl_raster_draw(raster,vbo,BL_VBO_POINTS);
            break;

            case RenderMode::Lines:
                bl_raster_draw(raster,vbo,BL_VBO_LINES);
            break;

            case RenderMode::Triangles:
                bl_raster_draw(raster,vbo,BL_VBO_TRIANGLES);
            break;

        }
        raster->main=bl_time_us();
        //bl_raster_flush_draw(raster);
        //bl_raster_update(raster);
            //bl_raster_update(raster);
        bl_raster_flush_draw(raster);
        auto t2b2 = std::chrono::steady_clock::now();
        bl_raster_flush_update(raster);

        if (request_data) {
            request_data=false;
            uint16_t depth = bl_texture_get_depth(raster->depth_buffer,rx,ry);
            cout<<"Depth at: "<<rx<<","<<ry<<": "<<depth<<endl;
        }
        
        auto t2c = std::chrono::steady_clock::now();
        
        time_raster_draw+=std::chrono::duration_cast<std::chrono::microseconds>(t2b2-t2b).count();
        time_raster_update+=std::chrono::duration_cast<std::chrono::microseconds>(t2c-t2b2).count();
        
        SDL_Rect rect;

        rect.x=0;
        rect.y=0;
        rect.w=WIDTH;
        rect.h=HEIGHT;

        auto t3 = std::chrono::steady_clock::now();
        
        SDL_UpdateTexture(texture,&rect,(void*)raster->color_buffer->data,WIDTH*sizeof(uint32_t));
        
        auto t4 = std::chrono::steady_clock::now();
        time_upload+=std::chrono::duration_cast<std::chrono::microseconds>(t4-t3).count();
        
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        auto t5 = std::chrono::steady_clock::now();
        time_present+=std::chrono::duration_cast<std::chrono::microseconds>(t5-t4).count();
        time_total+=std::chrono::duration_cast<std::chrono::microseconds>(t5-t0a).count();
        
        fps++;
        dfps=std::chrono::duration_cast<std::chrono::milliseconds>(t5-tfps).count();
        
        if (dfps>1000) {
        
            if (fps>max_fps) {
                max_fps=fps;
            }
            
            if (fps<min_fps) {
                min_fps=fps;
            }
            
            avg_fps.push_front(fps);
            
            if (avg_fps.size()>32) {
                avg_fps.pop_back();
            }
            
            int avg=0;
            auto it=avg_fps.begin();
            while(it!=avg_fps.end()) {
                avg+=*it;
                it++;
            }
            
            avg=avg/avg_fps.size();
        
            clog<<endl<<"****************"<<endl;
            
            clog<<"fps: "<<fps<<endl;
            clog<<"fps min: "<<min_fps<<endl;
            clog<<"fps max: "<<max_fps<<endl;
            clog<<"fps avg: "<<avg<<endl;
            print_time("input",time_input,fps);
            print_time("clear",time_clear,fps);
            print_time("draw",time_raster_draw,fps);
            print_time("update",time_raster_update,fps);
            print_time("upload",time_upload,fps);
            print_time("present",time_present,fps);
            
            clog<<"other: "<<(1000000-time_input-time_clear-time_raster_draw-time_raster_update-time_upload-time_present)/1000.0<<" ms"<<endl;
            clog<<"total: "<<time_total/1000.0<<" ms"<<endl;

            clog<<endl<<"workers:"<<endl;
            int num_workers = raster->draw_workers + raster->update_workers;
            for (int n=0;n<num_workers;n++) {
                int worker_type = raster->workers[n]->type;
                clog<<"["<<(int)worker_type<<"] wait "<<raster->workers[n]->time.wait<<" us, work "<<raster->workers[n]->time.work<<" us"<<" job started at "<<raster->workers[n]->time.start-raster->start<<" and ended at "<<raster->workers[n]->time.last-raster->start<<" us"<<endl;
                raster->workers[n]->time.wait=0;
                raster->workers[n]->time.work=0;
                if (worker_type == 1) {
                    clog<<"Chunks updated: "<<raster->workers[n]->update.chunks<<endl;
                }
            }
            
            clog<<"flush at "<<raster->main-raster->start<<" us"<<endl;
            
            dfps=0;
            fps=0;
            time_input=0;
            time_raster_draw=0;
            time_raster_update=0;
            time_clear=0;
            time_upload=0;
            time_present=0;
            time_total=0;
            
            tfps = std::chrono::steady_clock::now();
        }
    }
    
    
    

    bl_raster_delete(raster);
    
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
