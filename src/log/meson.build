sglog_src = files('sg_log.c')
sglog_inc = include_directories('.')
sglog_lib = static_library('sg_log',
                          sources: [sglog_src],
	                      dependencies: [extern_deps],
						  include_directories: [sglog_inc, extern_incs])
