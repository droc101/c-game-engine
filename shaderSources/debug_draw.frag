#version 460

layout (location = 0) flat in vec4 inColor;

layout (location = 0) out vec4 outColor;

void main() {
	if (inColor.a < 1 && int(gl_FragCoord.x + gl_FragCoord.y) % 2 == 0) {
		discard;
	}
	outColor = vec4(inColor.rgb, 1);
}
