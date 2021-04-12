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

struct objectInfo {
	mat4 transform;
	vec4 position;
};

layout(set = 0, binding = 2) buffer ObjectInfos_T {
	objectInfo objects[];
} ObjectInfos;

// A set for data that is constantly changing
layout(set = 1, binding = 0) uniform TransformUniform_T {
	mat4 mesh; // Mesh transform matrix
	mat4 view; // Relative to camera
	mat4 proj; // Relative to monitor
} TransformUniform;

layout(set = 0, binding = 3) uniform LightSource_T {
	vec4 pos;
	vec4 color;
} LightSource;

layout(location = 0) out vec3 v_Normal;
layout(location = 1) out vec2 v_TexCoord;
layout(location = 2) out vec4 v_Color;
layout(location = 3) out vec3 v_ToLight;


void main() {
	Vertex v = Vertices.vertices[gl_VertexIndex];
	vec4 wPosition  = vec4(v.vert[0], v.vert[1], v.vert[2], 0);
	wPosition      += ObjectInfos.objects[gl_InstanceIndex].position;
	mat4 wTransform = TransformUniform.mesh * ObjectInfos.objects[gl_InstanceIndex].transform;
	wPosition      *= wTransform;
	v_Normal        = vec3(v.norm[0], v.norm[1], v.norm[2]);
	v_Normal        = normalize(vec4(vec4(v_Normal, 0.0) * wTransform).xyz); 
	v_TexCoord      = vec2(v.tex[0], v.tex[1]);
	gl_Position     = TransformUniform.proj * TransformUniform.view * wPosition;
	v_Color         = LightSource.color;
	v_ToLight       = LightSource.pos.xyz - wPosition.xyz;
}
