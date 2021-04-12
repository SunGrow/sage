#version 460
#extension GL_ARB_separate_shader_objects : enable

struct Vertex {
	float vert[3];
	float norm[3];
	float tex[2];
};

// A set for constant data that does not change between frames
struct objectInfo {
	mat4 transform;
	vec4 position;
};

layout(set = 0, binding = 0) readonly buffer Vertices_T {
	Vertex vertices[];
} Vertices;
layout(set = 0, binding = 1) buffer ObjectInfos_T {
	objectInfo objects[];
} ObjectInfos;

// A set for data that is constantly changing
layout(set = 1, binding = 0) uniform TransformUniform_T {
	mat4 mesh; // Mesh transform matrix
	mat4 view; // Relative to camera
	mat4 proj; // Relative to monitor
} TransformUniform;

layout(set = 0, binding = 2) uniform LightSource_T {
	vec4 pos;
	vec4 color;
} LightSource;

layout(location = 0) out vec4 v_Color;

void main() {
	Vertex v = Vertices.vertices[gl_VertexIndex];
	vec4 wPosition  = vec4(v.vert[0], v.vert[1], v.vert[2], 0);
	wPosition      += ObjectInfos.objects[gl_InstanceIndex].position;

	mat4 wTransform = TransformUniform.mesh * ObjectInfos.objects[gl_InstanceIndex].transform;
	wPosition      *= wTransform;
	gl_Position     = TransformUniform.proj * TransformUniform.view * wPosition;
	v_Color         = LightSource.color;
}
