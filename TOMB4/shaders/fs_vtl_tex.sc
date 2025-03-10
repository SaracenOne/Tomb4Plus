$input v_color0, v_color1, v_texcoord0, v_texcoord1

#include "common.sh"
#include "fog.sh"

SAMPLER2D(s_texColor,  0);
uniform vec4 u_fogColor;
uniform vec4 u_volumetricFogColor;
uniform vec4 u_fogParameters;

void main()
{
	vec4 texColor_var = texture2D(s_texColor, v_texcoord0).rgba;

	// Vertex colour is swizzled.
	vec4 vCol0 = vec4(v_color0.b, v_color0.g, v_color0.r, 1.0);
	vec4 vCol1 = vec4(v_color1.b, v_color1.g, v_color1.r, v_color1.a);

	vec4 outCol = texColor_var * vCol0;

	// Blend fog
	float depth = (gl_FragCoord.z / gl_FragCoord.w);
	float fogFactor = CalculateFogFactor(depth, u_fogParameters.x, u_fogParameters.y);
	outCol.rgb = mix(outCol.rgb, u_fogColor.rgb, fogFactor);

	// Volumetric Fog
	outCol.rgb = mix(outCol.rgb, u_volumetricFogColor.rgb, 1.0 - vCol1.a) + vCol1.rgb;

	gl_FragColor = outCol;
}
