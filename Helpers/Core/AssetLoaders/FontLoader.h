//
// Created by droc101 on 7/23/25.
//

#ifndef FONTLOADER_H
#define FONTLOADER_H

#include "../../../defines.h"

/**
 * Load a font from an asset
 * @param asset The asset to load the font from
 * @return The loaded font, or NULL if it failed
 * @note This pointer is not tracked and must be freed manually.
 */
Font *LoadFont(const char *asset);

#endif //FONTLOADER_H
