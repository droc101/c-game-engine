#version 330 core

in vec2 UV; // UV coordinates of the fragment

out vec4 COLOR; // Output color of the fragment

uniform sampler2D alb; // Albedo texture of the shadow

layout (std140) uniform SharedUniforms
{
	mat4 worldViewMatrix;
	vec3 fogColor;
	float fogStart;
	float fogEnd;
	float cameraYaw;
} uniforms;

void main() {
	COLOR = texture(alb, UV).rgba;
	COLOR.a *= 0.5;
	if (COLOR.a < 0.1) discard;

	float fog_factor = clamp((gl_FragCoord.z / gl_FragCoord.w - uniforms.fogStart) / (uniforms.fogEnd - uniforms.fogStart), 0.0, 1.0);
	COLOR.rgb = mix(COLOR.rgb, uniforms.fogColor, fog_factor);
}