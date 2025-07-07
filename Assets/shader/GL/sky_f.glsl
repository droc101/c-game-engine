#version 330 core

in vec2 UV; // UV coordinates of the fragment

out vec4 COLOR; // Output color of the fragment

uniform sampler2D alb;

void main() {
	COLOR = texture(alb, UV).rgba;
}