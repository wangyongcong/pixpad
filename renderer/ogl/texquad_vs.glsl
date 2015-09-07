#version 330

uniform mat4 proj;
in vec2 pos;
out vec2 f_uv;

void main()
{
	gl_Position = proj * vec4(pos, 0.0, 1.0);
	f_uv = pos;
}
