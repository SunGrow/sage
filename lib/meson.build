mathrun_lib = meson.get_compiler('c').find_library('m', required: false)
crun_lib = meson.get_compiler('c').find_library('c', required: false)
pthreadrun_lib = meson.get_compiler('c').find_library('pthread', required: false)

sage_lib = shared_library('sage',
                          sources: sage_src,
						  include_directories : [extern_incs, sage_incs],
						  link_with: [extern_libs],
						  dependencies: [mathrun_lib, crun_lib, pthreadrun_lib]
						 )
