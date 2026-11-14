#version 460

#include "../include/shared.inc.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) flat in vec3 inCenter;
layout(location = 2) flat in float inRadius;
layout(location = 3) flat in vec3 inColor;
layout(location = 4) flat in uint inKeptPixelIndex;

layout(location = 0) out vec4 outColor;

void main() {
    // The mesh fully encloses the sphere, so the actual sphere is found by intersecting it with this pixel's view ray. Any
    // point on the mesh lies on the view ray, so the ray direction is exact even though the mesh is faceted.
    const vec3 rayDirection = normalize(inPosition - camera.position);
    // The angle covered by a single pixel. Computed before any discard so that the derivatives stay well defined.
    const float pixelAngle = max(length(dFdx(rayDirection)), length(dFdy(rayDirection)));

    const vec3 toCenter = inCenter - camera.position;
    const float centerDistance = length(toCenter);
    if (centerDistance > 150) {
        discard;
    }
    const float closestApproach = dot(toCenter, rayDirection);
    const float asd = inRadius * inRadius - (centerDistance * centerDistance - closestApproach * closestApproach);
    if (asd < 0) {
        discard;
    }

    const bool insideSphere = centerDistance < inRadius;
    const bool pixelIsEdge = !insideSphere &&
                             abs(dot(normalize(camera.position + rayDirection * (insideSphere ? closestApproach + sqrt(asd) : closestApproach - sqrt(asd)) - inCenter), rayDirection)) < 0.2225 &&
                             (asin(inRadius / centerDistance) - acos(clamp(closestApproach / centerDistance, -1.0, 1.0))) / max(pixelAngle, EPSILON) < 2;
    if (!pixelIsEdge) {
        const uvec2 pixel = uvec2(gl_FragCoord.xy) % 2;
        if (pixel.x + pixel.y * 2 != inKeptPixelIndex) {
            discard;
        }
    }

    outColor = vec4(inColor * (pixelIsEdge ? 1.0 : 0.75), 1);
}
