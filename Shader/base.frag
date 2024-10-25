#version 450

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec3 in_arm;
layout(location = 2) in vec2 in_motionVector;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_normal;
layout(location = 2) out vec4 out_arm;
layout(location = 3) out vec2 out_motion;

void main()
{
	out_color = vec4(1.0, 1.0, 0.0, 1.0);
	out_normal = vec4(in_normal, 1.0);
	out_arm = vec4(in_arm, 1.0);
	out_motion = vec2(in_motionVector);
}