$input v_color0, v_color1, v_texcoord0, v_texcoord1

#include "common.sh"

void main()
{
	gl_FragColor = vec4(v_color0.b, v_color0.g, v_color0.r, v_color0.a);
}
