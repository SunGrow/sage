mathrun_lib = meson.get_compiler('c').find_library('m', required: false)
crun_lib = meson.get_compiler('c').find_library('c', required: false)
pthreadrun_lib = meson.get_compiler('c').find_library('pthread', required: false)

dl_lib = meson.get_compiler('c').find_library('dl', required: false)
extern_deps += [mathrun_lib, crun_lib, pthreadrun_lib, dl_lib]

vk_inc = include_directories('Vulkan-Headers/include')
extern_incs += vk_inc

cgltf_inc = include_directories('cgltf')
extern_incs += cgltf_inc

fobj_inc  = include_directories('fast_obj')
extern_incs += fobj_inc

volk_inc  = include_directories('volk')
volk_lib  = static_library('volk',
                    sources: 'volk/volk.c',
					dependencies: [dl_lib],
			        include_directories: [volk_inc, vk_inc])
volk_dep = declare_dependency(link_with : volk_lib,
							  include_directories: volk_inc)
extern_incs += volk_inc
extern_deps += volk_dep

vma_inc   = include_directories('VulkanMemoryAllocator/src')
vma_lib   = static_library('vma',
                    sources: 'vma_implement.cpp',
	                dependencies: [volk_dep],
			        include_directories: [vma_inc, vk_inc])
vma_dep   = declare_dependency(link_with : vma_lib,
							   include_directories: vma_inc)
extern_incs += vma_inc
extern_deps += vma_dep

logc_inc  = include_directories('log.c/src')
logc_lib  = static_library('log',
                    sources: 'log.c/src/log.c',
			        include_directories: logc_inc,
			        c_args: '-DLOG_USE_COLOR')
logc_dep = declare_dependency(link_with : logc_lib,
							  include_directories: logc_inc)

extern_incs += logc_inc
extern_deps += logc_dep

meshopt_files = files(['meshoptimizer/src/allocator.cpp', 'meshoptimizer/src/clusterizer.cpp', 'meshoptimizer/src/indexcodec.cpp', 'meshoptimizer/src/indexgenerator.cpp', 'meshoptimizer/src/overdrawanalyzer.cpp', 'meshoptimizer/src/overdrawoptimizer.cpp', 'meshoptimizer/src/simplifier.cpp', 'meshoptimizer/src/spatialorder.cpp', 'meshoptimizer/src/stripifier.cpp', 'meshoptimizer/src/vcacheanalyzer.cpp', 'meshoptimizer/src/vcacheoptimizer.cpp', 'meshoptimizer/src/vertexcodec.cpp', 'meshoptimizer/src/vertexfilter.cpp', 'meshoptimizer/src/vfetchanalyzer.cpp', 'meshoptimizer/src/vfetchoptimizer.cpp'])
meshopt_inc = include_directories('meshoptimizer/src')
meshopt_lib = static_library('meshoptimizer',
                    sources: meshopt_files,
			        include_directories: meshopt_inc,
			        )
meshopt_dep = declare_dependency(link_with : meshopt_lib,
                                 include_directories: meshopt_inc)
extern_incs += meshopt_inc
extern_deps += meshopt_dep
 
