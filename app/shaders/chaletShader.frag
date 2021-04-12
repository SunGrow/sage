#version 460
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in vec4 v_Color;
layout(location = 3) in vec3 v_ToLight;

layout(set = 0, binding = 1) uniform sampler2D u_TexSampler;

void main() {
	vec4 depth = vec4(gl_FragCoord.z,gl_FragCoord.z,gl_FragCoord.z, 1.0);
	vec4 tex = texture(u_TexSampler, v_TexCoord);
	float lighted = dot(v_Normal, v_ToLight);
	lighted = min(max(lighted, 0.2), 1.0);
	// No transfer on tex coord and normal
	o_Color  = tex * (lighted * v_Color) * depth;
}
