vec4 vt_transform(vec4 vertex)
{
	float w = 1.0 / vertex.w;
	vec4 transformed_vertex = vec4(vertex.x * w, vertex.y * w, vertex.z * w, w);
	
	return transformed_vertex;
}
