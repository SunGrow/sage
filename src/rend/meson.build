sgrend_srcfiles = ['sg_rend.c', 'debug/sg_rend_debug.c', 'device/sg_rend_device.c', 'surface/sg_rend_surface.c', 'allocator/sg_rend_allocator.c']
sgrend_src = files(sgrend_srcfiles)
sgrend_inc = include_directories('.')
sgrend_lib = library('sg_rend',
                          sources: sgrend_src,
	                      dependencies: [extern_deps],
						  include_directories: [sgrend_inc, sgbase_inc, sgmath_inc, extern_incs])
