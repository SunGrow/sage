#version 460

struct Vertex {
	float vx, vy, vz;
	float nx, ny, nz;
	float tu, tv;
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


void main() {
	Vertex v = Vertices.vertices[gl_VertexIndex];
	vec3 position = vec3(v.vx, v.vy, v.vz);
	vec3 normal = vec3(v.nx, v.ny, v.nz);
	vec2 texcoord = vec2(v.tu, v.tv);
	gl_Position = TransformUniform.proj * TransformUniform.view * TransformUniform.mesh * vec4(position, 1.0);
}
