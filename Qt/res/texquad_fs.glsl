#version 330

uniform sampler2D basemap;
in vec2 f_uv;

void main()
{
	gl_FragColor = vec4(texture2D(basemap, f_uv).rgb, 1.0);
}
