sgscene_src = files(['sg_scene.c'])
sgscene_inc = include_directories('.')
sgscene_lib = library('sg_scene',
                          sources: [sgscene_src],
	                      dependencies: [extern_deps],
						  link_with:[sgmath_lib],
						  include_directories: [sgbase_inc, sgscene_inc, extern_incs, sgmath_inc])
