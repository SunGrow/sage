vma_inc   = include_directories('VulkanMemoryAllocator/src')

cgltf_inc = include_directories('cgltf')

fobj_inc  = include_directories('fast_obj')

flecs_inc = include_directories('flecs')

logc_inc  = include_directories('log.c/src')
logc_lib  = static_library('log', sources: 'log.c/src/log.c', include_directories: logc_inc)

volk_inc  = include_directories('volk')


extern_incs = [vma_inc, cgltf_inc, fobj_inc, flecs_inc, logc_inc, volk_inc]
extern_libs = [logc_lib]
