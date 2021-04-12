#version 460
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec4 v_Color;

float radius = 100;
void main() {
    o_Color = v_Color;
}
