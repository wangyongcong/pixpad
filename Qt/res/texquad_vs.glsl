#version 330

uniform mat4 projection;
in vec2 pos;
out vec2 f_uv;

void main()
{
	gl_Position = projection * vec4(pos, 0.0, 1.0);
	f_uv = pos;
}
