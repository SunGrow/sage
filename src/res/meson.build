sgres_src = files(['sg_mesh.c'])
sgres_inc = include_directories('.')
sgres_lib = library('sg_res',
                          sources: [sgres_src],
	                      dependencies: [extern_deps],
						  include_directories: [sgbase_inc, sgres_inc, extern_incs, sgmath_inc])
