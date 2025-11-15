//
// Created by droc101 on 11/14/2025.
//

#ifndef GAME_FILESYSTEM_H
#define GAME_FILESYSTEM_H

/**
 * Get the directory a file resides in
 * @param fileName The file to get the directory of
 * @return The file's directory, or NULL on failure
 * @warning The returned path may be relative to the current working directory
 */
char *GetDirectoryOfFile(char *fileName);

/**
 * Change the process working directory
 * @param dirName The new working directory
 */
void ChangeDirectory(const char *dirName);

#endif //GAME_FILESYSTEM_H
