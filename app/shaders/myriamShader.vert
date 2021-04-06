#version 460
#extension GL_ARB_separate_shader_objects : enable

struct Vertex {
	float vert[3];
	float norm[3];
	float tex[2];
};

// A set for constant data that does not change between frames
layout(set = 0, binding = 0) readonly buffer Vertices_T {
	Vertex vertices[];
} Vertices;

// A set for data that is constantly changing
layout(set = 1, binding = 0) uniform TransformUniform_T {
	mat4 mesh; // Mesh transform matrix
	mat4 view; // Relative to camera
	mat4 proj; // Relative to monitor
} TransformUniform;

layout(location = 0) out vec3 v_Normal;
layout(location = 1) out vec2 v_TexCoord;
layout(location = 2) out vec4 v_Color;


void main() {
	Vertex v = Vertices.vertices[gl_VertexIndex];
	vec3 position   = vec3(v.vert[0], v.vert[1], v.vert[2]);
	v_Normal   = vec3(v.norm[0], v.norm[1], v.norm[2]);
	v_TexCoord = vec2(v.tex[0], v.tex[1]);
	gl_Position     = TransformUniform.proj * TransformUniform.view * TransformUniform.mesh * vec4(position, 1.0);
	v_Color         = vec4(0.9, 0.7, 0.2, 1.0);
}
