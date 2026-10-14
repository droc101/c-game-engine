// Include only. This file will not compile as a standalone module.

#include "shared.inc.glsl"

layout(location = 10) in vec2 inUV;
layout(location = 11) flat in uint inTextureIndex;

layout(location = 0) out vec4 outColor;

void getTextureColor() {
#ifdef UNSHADED
#   define SHOULD_SAMPLE_TEXTURE DEBUG_RENDERING != DEBUG_RENDERING_UNTEXTURED
#else
#   define SHOULD_SAMPLE_TEXTURE DEBUG_RENDERING != DEBUG_RENDERING_UNTEXTURED && DEBUG_RENDERING != DEBUG_RENDERING_ONLY_LIGHTING && DEBUG_RENDERING != DEBUG_RENDERING_NO_LIGHT_FALLOFF
#endif
    if (SHOULD_SAMPLE_TEXTURE) {
    	outColor = texture(textureSampler[nonuniformEXT(inTextureIndex)], inUV);
		outColor.a = 1.0;
	} else {
		outColor = vec4(1);
	}
#undef SHOULD_SAMPLE_TEXTURE
}