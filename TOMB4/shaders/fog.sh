float CalculateFogFactor(float depth, float fog_start, float fog_end)
{
	float fogStart = (fog_start) / 2.0;
	float fogEnd = (fog_end) / 2.0;

	float fogDistance = depth;
	float fogFactor = (fogEnd - fogDistance) / (fogEnd - fogStart);
	fogFactor = clamp(fogFactor, 0.0, 1.0);

	return 1.0 - fogFactor;
}