sgwindow_src = files(['sg_window.c'])
sgwindow_inc = include_directories('.')
sgwindow_lib = static_library('sg_window',
                          sources: [sgwindow_src],
	                      dependencies: [extern_deps],
						  link_with: [],
						  include_directories: [sgbase_inc, extern_incs, sgwindow_inc])
