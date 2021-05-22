sgcamera_src = files(['sg_camera.c'])
sgcamera_inc = include_directories('.')
sgcamera_lib = static_library('sg_camera',
                          sources: [sgcamera_src],
	                      dependencies: [extern_deps],
						  link_with:[sgmath_lib],
						  include_directories: [sgbase_inc, sgcamera_inc, extern_incs, sgmath_inc])
