//
// Created by droc101 on 5/19/26.
//

#ifndef GAME_KVLFILE_H
#define GAME_KVLFILE_H

#include <engine/structs/KVList.h>
#include <stdbool.h>

/**
 * Read a KvlFile from a path
 * @param path The path to read from
 * @param output The KvList to read into
 * @return Whether the file was successfully read
 */
bool ReadKvlFile(const char *path, KvList output);

/**
 * Write a KvlFile to a given path
 * @param path The path to write to
 * @param input The KvList to write
 * @return Whether the file was successfully written
 */
bool WriteKvlFile(const char *path, KvList input);

#endif //GAME_KVLFILE_H
