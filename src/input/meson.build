sginput_src = files(['sg_input.c'])
sginput_inc = include_directories('.')
sginput_lib = static_library('sg_input',
                          sources: [sginput_src],
	                      dependencies: [extern_deps],
						  link_with: [],
						  include_directories: [sgbase_inc, extern_incs, sginput_inc])
