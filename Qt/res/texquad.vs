#version 330

uniform mat4 projection;
in vec3 position;
in vec2 uv;
out vec2 f_uv;

void main()
{
	gl_Position = projection * vec4(position, 1.0);
	f_uv = uv;
}
