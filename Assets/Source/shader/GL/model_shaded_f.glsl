#version 330 core

in vec2 UV; // UV coordinates of the fragment
in float NORMAL_Z; // Z component of the normal vector of the fragment

out vec4 COLOR; // Output color of the fragment

uniform sampler2D alb;
uniform vec4 albColor;

void main() {
	if (texture(alb, UV).a * albColor.a < 0.5) {
		discard; // Discard the fragment if the alpha is less than 0.5
	}

	COLOR.a = 1.0;
	COLOR.rgb = texture(alb, UV).rgb * albColor.rgb;

	// Apply easing to shading
	float shading = NORMAL_Z == 1 ? 1 : 1 - pow(2, -10 * NORMAL_Z);

	shading = max(0.6, shading);

	COLOR.rgb *= vec3(shading);
}