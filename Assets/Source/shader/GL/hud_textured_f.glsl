#version 330 core

/**
 * This shader is used to render a textured quad with a tint and optional region.
 */

// Input from vertex shader
in vec2 UV;

// Output to fragment shader
out vec4 COLOR;

// Uniforms
uniform sampler2D alb;
uniform vec4 col;
uniform vec4 region;  // region.xy = start, region.zw = size, if region.x == -1.0, no region

void main() {
	vec2 uv = UV;

	// If region.x != -1.0, apply region-based UV adjustment
	if (region.x != -1.0) {
		ivec2 tex_size = textureSize(alb, 0);
		vec2 start = region.xy / tex_size;
		vec2 size = region.zw / tex_size;

		uv = start + UV * size;
	}

	COLOR = texture(alb, uv) * col;
}
