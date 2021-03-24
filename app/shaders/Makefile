# svkr shader render makefile

GLSL = glslc
GLSLFLAGS = --target-env=vulkan1.2

GLSRC = $(wildcard *.vert) $(wildcard *.frag)

all: options shaders

options:
	@echo shader build options:
	@echo "GLSLC      = ${GLSL}"
	@echo "GLSLCFLAGS = ${GLSLFLAGS}"

shaders: options
	${GLSL} -c ${GLSRC} ${GLSLFLAGS}

clean:
	rm -f *.spv
