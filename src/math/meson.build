sgmath_src = files('sg_math.c')
sgmath_inc = include_directories('.')
sgmath_lib = static_library('sg_math',
                          sources: sgmath_src,
	                      dependencies: [extern_deps],
						  include_directories: [sgmath_inc, sgbase_inc])
