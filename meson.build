project('sage', 'c',
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


subdir('src')
subdir('lib')
subdir('app')

