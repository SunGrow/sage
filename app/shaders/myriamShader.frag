#version 460
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in vec4 v_Color;

layout(set = 0, binding = 1) uniform sampler2D u_SkinSampler;
layout(set = 0, binding = 2) uniform sampler2D u_HairSampler;
layout(set = 0, binding = 3) uniform sampler2D u_ClothSampler;
layout(set = 0, binding = 4) uniform sampler2D u_BodySampler;

void main() {
	vec4 depth = vec4(gl_FragCoord.z,gl_FragCoord.z,gl_FragCoord.z, 1.0);
	vec4 skinTex  = texture(u_SkinSampler,  v_TexCoord);
	vec4 hairTex  = texture(u_HairSampler,  v_TexCoord);
	vec4 clothTex = texture(u_ClothSampler, v_TexCoord);
	vec4 bodyTex = texture(u_BodySampler, v_TexCoord);
	// No transfer on tex coord and normal
	o_Color  = vec4(bodyTex[0], bodyTex[1], bodyTex[2], (skinTex[3] + hairTex[3] + clothTex[3])) * depth;
}
