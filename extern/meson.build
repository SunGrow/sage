
# TODO vma.cpp
vma_inc   = include_directories('VulkanMemoryAllocator/src')
extern_incs += vma_inc

cgltf_inc = include_directories('cgltf')
extern_incs += cgltf_inc

fobj_inc  = include_directories('fast_obj')
extern_incs += fobj_inc

# TODO volk.c
volk_inc  = include_directories('volk')
volk_lib  = static_library('volk',
                    sources: 'volk/volk.c',
			        include_directories: volk_inc)
volk_dep = declare_dependency(link_with : volk_lib,
							  include_directories: volk_inc)
extern_incs += volk_inc
extern_deps += volk_dep

logc_inc  = include_directories('log.c/src')
logc_lib  = library('log',
                    sources: 'log.c/src/log.c',
			        include_directories: logc_inc,
			        c_args: '-DLOG_USE_COLOR')
logc_dep = declare_dependency(link_with : logc_lib,
							  include_directories: logc_inc)

extern_incs += logc_inc
extern_deps += logc_dep

mathrun_lib = meson.get_compiler('c').find_library('m', required: false)
crun_lib = meson.get_compiler('c').find_library('c', required: false)
pthreadrun_lib = meson.get_compiler('c').find_library('pthread', required: false)

dl_lib = meson.get_compiler('c').find_library('dl', required: false)

extern_deps += [mathrun_lib, crun_lib, pthreadrun_lib, dl_lib]
