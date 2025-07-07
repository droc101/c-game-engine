#version 330 core

in vec2 UV; // UV coordinates of the fragment
in vec3 NORMAL; // Normal of the fragment

out vec4 COLOR; // Output color of the fragment

uniform sampler2D alb;
uniform vec4 albColor;

void main() {
	if (texture(alb, UV).a * albColor.a < 0.5) {
		discard; // Discard the fragment if the alpha is less than 0.5
	}

	COLOR.a = 1.0;
	COLOR.rgb = texture(alb, UV).rgb * albColor.rgb;

	// Calculate shading based on the normal, assuming a light direction of (0, 0, 1)
	float shading = dot(NORMAL, vec3(0, 0, 1));

	// Apply easing to shading
	shading = shading == 1 ? 1 : 1 - pow(2, -10 * shading);

	shading = max(0.6, shading);

	COLOR.rgb *= vec3(shading);
}