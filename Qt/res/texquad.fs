#version 330

uniform sampler2D tex;
in vec2 f_uv;

void main()
{
	gl_FragColor = texture2D(tex, f_uv);
}
