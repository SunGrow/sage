sgbase_src = files('sage_base.c')
sgbase_inc = include_directories('.')
sgbase_lib = library('sg_base',
                          sources: [sgbase_src],
	                      dependencies: [extern_deps],
						  include_directories: [sgbase_inc, extern_incs])
