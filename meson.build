project('sage', 'c', 'cpp',
  version: '0.1.0',
  license: 'MIT',
  default_options: ['warning_level=3', 'c_std=c17'])
 
base_inc = include_directories('include')

# extern
extern_deps = []
extern_incs = []

subdir('extern')

flecs_dep = dependency('flecs', fallback : ['flecs', 'flecs_dep'])
extern_deps += flecs_dep

cmake = import('cmake')

glfw_subproj = cmake.subproject('glfw')
glfw_inc = glfw_subproj.include_directories('glfw')
glfw_dep = glfw_subproj.dependency('glfw')
extern_deps += glfw_dep


# Scripts
base_dir = meson.current_source_dir()
scripts_dir = join_paths(base_dir, 'scripts')
postinstallcopy = join_paths(scripts_dir, 'foldercopy.py')

projroot_dir = meson.project_source_root()
buildroot_dir = meson.project_build_root()

projshader_dir = join_paths(projroot_dir, 'shaders')
buildapp_dir = join_paths(buildroot_dir, 'app')
buildshader_dir = join_paths(buildapp_dir, 'shaders')

run_command('python', postinstallcopy, projshader_dir, buildshader_dir)
#


subdir('src')
subdir('lib')
subdir('app')
subdir('assetsys')

