sgjobs_src = files(['sg_jobs.c'])
sgjobs_inc = include_directories('.')
sgjobs_lib = static_library('sg_window',
                          sources: [sgjobs_src],
	                      dependencies: [extern_deps],
						  link_with: [],
						  include_directories: [sgbase_inc, sgjobs_inc, extern_incs])
