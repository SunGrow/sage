project('sage', 'c', 'cpp',
  version: '0.1.0',
  license: 'MIT')
 
base_inc = include_directories('include')

# extern
extern_deps = []
extern_incs = []

subdir('extern')

flecs_inc = include_directories('subprojects/flecs/include')
flecs_dep = dependency('flecs', fallback : ['flecs', 'flecs_dep'])
extern_deps += flecs_dep

cmake = import('cmake')

glfw_subproj = cmake.subproject('glfw')
glfw_inc = glfw_subproj.include_directories('glfw')
glfw_dep = glfw_subproj.dependency('glfw')
extern_deps += glfw_dep
#
#base_dir = meson.current_source_dir()
#scripts_dir = join_paths(base_dir, 'scripts')
#foldercopier_filename = join_paths(base_dir, 'foldercopy.py')
#
#message(scripts_dir)
#foldercopier = find_program('foldercopy.py', dirs: scripts_dir)
#
#a_p = custom_target('res',
#    input : 'res',
#    output : 'app/res',
#    command : [foldercopier, '@INPUT@', '@OUTPUT@'])
#a_p = custom_target('shaders',
#    input : 'shaders',
#    output : 'res/shaders',
#    command : [foldercopier, '@INPUT@', '@OUTPUT@'])
subdir('src')
subdir('lib')
subdir('app')

