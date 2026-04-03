//
// Created by droc101 on 4/3/26.
//

#ifndef GAME_STD140_H
#define GAME_STD140_H

#include <cglm/types.h>
#include <engine/structs/Color.h>
#include <stdint.h>

/// Unaligned mat4 for STD140 layout
typedef vec4 mat4_std140[4];

/// Align a variable to a multiple of sizeof(float)
#define STD140_ALIGN(N) _Alignas(N * sizeof(float))
/// STD140 structs require this
#define STD140 __attribute__((__packed__))

/// 4N aligned mat4
#define STD140_MAT4 STD140_ALIGN(4) mat4_std140
/// 4N aligned vec4
#define STD140_VEC4 STD140_ALIGN(4) vec4
/// 4N aligned Color
#define STD140_COLOR STD140_ALIGN(4) Color
/// 4N aligned vec3
#define STD140_VEC3 STD140_ALIGN(4) vec3
/// 4N aligned ivec4
#define STD140_IVEC4 STD140_ALIGN(4) ivec4
/// 4N aligned ivec3
#define STD140_IVEC3 STD140_ALIGN(4) ivec3

/// 2N aligned vec2
#define STD140_VEC2 STD140_ALIGN(2) vec2
/// 2N aligned ivec2
#define STD140_IVEC2 STD140_ALIGN(2) ivec2

/// 1N aligned float
#define STD140_FLOAT STD140_ALIGN(1) float
/// 1N aligned int32_t
#define STD140_INT STD140_ALIGN(1) int32_t
/// 1N aligned uint32_t
#define STD140_UINT STD140_ALIGN(1) uint32_t
/// 1N aligned bool
#define STD140_BOOL STD140_ALIGN(1) bool

#endif //GAME_STD140_H
