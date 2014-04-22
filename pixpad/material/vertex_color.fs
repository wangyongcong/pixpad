#version 330

in vec3 frag_color;

void main()
{
	gl_FragColor.rgb = frag_color;
}