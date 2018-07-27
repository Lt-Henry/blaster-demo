
sdl=dependency('sdl2')
glew=dependency('glew')
blaster_dep=blaster.get_variable('blaster')

executable('blaster-demo', ['main.cpp'],
    cpp_args:'-std=c++11',
    dependencies:[sdl,blaster_dep,glew]
    )
