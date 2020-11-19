sage_lib = shared_library('sage',
						  include_directories : [extern_incs, sage_incs],
						  link_with: [sage_libs],
						  dependencies: [extern_deps])

sage_dep = declare_dependency( include_directories : [extern_incs, sage_incs],
	                          link_with : [sage_libs],
						      dependencies: [extern_deps])
