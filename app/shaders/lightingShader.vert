#version 460
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 4) uniform LightSource_T {
	vec4 pos;
	vec4 color;
} LightSource;

layout(location = 2) out vec4 v_Color;

void main() {
	v_Color         = LightSource.color;
}
