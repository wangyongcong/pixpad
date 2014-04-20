#version 330

in vec3 position;
in vec3 color;
uniform mat4 projection;
uniform mat4 transform;

out vec3 frag_color;

void main()
{
	gl_Position = projection*transform*vec4(position,1.0);
	frag_color = color;
}
