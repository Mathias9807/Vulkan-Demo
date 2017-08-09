#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex {
	vec4 gl_Position;
};

layout(location = 0) out vec3 color;

vec3 positions[3] = vec3[](
	vec3(-1, 1, 0),
	vec3(1,	1, 0),
	vec3(0, -1, 0)
);

// vec2 positions[3] = vec2[](
// 	vec2(-0.7 * 10,	0.7 * 10),
// 	vec2(0.7 * 10,	0.7 * 10),
// 	vec2(0.0 * 10,	-0.7 * 10)
// );

vec3 colors[] = vec3[](
	vec3(1, 0, 0),
	vec3(0, 1, 0),
	vec3(0, 0, 1)
);

void main() {
	gl_Position = vec4(positions[gl_VertexIndex], 1.0);

	color = colors[gl_VertexIndex];
}
