//
// Created by droc101 on 5/28/25.
//

#ifndef KVLIST_H
#define KVLIST_H
#include "../../defines.h"

/**
 * Creates a key-value list.
 * @param list The list to create.
 */
void KvListCreate(KvList *list);

/**
 * Destroys a key-value list, freeing all memory associated with it.
 * @param list The list to destroy.
 */
void KvListDestroy(KvList *list);

/**
 * Deletes a key from a key-value list.
 * @param list The list to delete from.
 * @param key The key to delete.
 */
void KvDelete(KvList *list, const char *key);

/**
 * Get the length of the key-value list (number of keys/values).
 * @param list The list to get the length of.
 * @return The number of keys/values in the list.
 */
size_t KvListLength(const KvList *list);

/**
 * Check if a key exists in the key-value list.
 * @param list The list to check.
 * @param key The key to check for.
 * @return Whether the key exists in the list.
 */
bool KvListHas(const KvList *list, const char *key);

/**
 * Get the type of a key in the key-value list.
 * @param list The list to check.
 * @param key The key to check for.
 * @return The type of the key, or PARAM_TYPE_NONE if it does not exist.
 */
ParamType KvGetType(const KvList *list, const char *key);

#pragma region Getters

/**
 * Get a byte value from the key-value list.
 * @param list The list to get the value from.
 * @param key The key to get the value for.
 * @param defaultValue The default value to return if the key does not exist.
 * @return The byte value associated with the key, or the default value if the key does not exist.
 */
byte KvGetByte(const KvList *list, const char *key, byte defaultValue);

/**
 * Get an integer value from the key-value list.
 * @param list The list to get the value from.
 * @param key The key to get the value for.
 * @param defaultValue The default value to return if the key does not exist.
 * @return The integer value associated with the key, or the default value if the key does not exist.
 */
int KvGetInt(const KvList *list, const char *key, int defaultValue);

/**
 * Get a float value from the key-value list.
 * @param list The list to get the value from.
 * @param key The key to get the value for.
 * @param defaultValue The default value to return if the key does not exist.
 * @return The float value associated with the key, or the default value if the key does not exist.
 */
float KvGetFloat(const KvList *list, const char *key, float defaultValue);

/**
 * Get a boolean value from the key-value list.
 * @param list The list to get the value from.
 * @param key The key to get the value for.
 * @param defaultValue The default value to return if the key does not exist.
 * @return The boolean value associated with the key, or the default value if the key does not exist.
 */
bool KvGetBool(const KvList *list, const char *key, bool defaultValue);

/**
 * Get a string value from the key-value list.
 * @param list The list to get the value from.
 * @param key The key to get the value for.
 * @param defaultValue The default value to return if the key does not exist.
 * @return The string value associated with the key, or the default value if the key does not exist.
 */
const char *KvGetString(const KvList *list, const char *key, const char *defaultValue);

/**
 * Get a Color value from the key-value list.
 * @param list The list to get the value from.
 * @param key The key to get the value for.
 * @param defaultValue The default value to return if the key does not exist.
 * @return The Color value associated with the key, or the default value if the key does not exist.
 */
Color KvGetColor(const KvList *list, const char *key, Color defaultValue);

#pragma endregion

#pragma region Setters

/**
 * Set a byte value in the key-value list.
 * @param list The list to set the value in.
 * @param key The key to set the value for.
 * @param value The byte value to set.
 */
void KvSetByte(KvList *list, const char *key, byte value);

/**
 * Set an integer value in the key-value list.
 * @param list The list to set the value in.
 * @param key The key to set the value for.
 * @param value The integer value to set.
 */
void KvSetInt(KvList *list, const char *key, int value);

/**
 * Set a float value in the key-value list.
 * @param list The list to set the value in.
 * @param key The key to set the value for.
 * @param value The float value to set.
 */
void KvSetFloat(KvList *list, const char *key, float value);

/**
 * Set a boolean value in the key-value list.
 * @param list The list to set the value in.
 * @param key The key to set the value for.
 * @param value The boolean value to set.
 */
void KvSetBool(KvList *list, const char *key, bool value);

/**
 * Set a string value in the key-value list.
 * @param list The list to set the value in.
 * @param key The key to set the value for.
 * @param value The string value to set.
 */
void KvSetString(KvList *list, const char *key, const char *value);

/**
 * Set a Color value in the key-value list.
 * @param list The list to set the value in.
 * @param key The key to set the value for.
 * @param value The Color value to set.
 */
void KvSetColor(KvList *list, const char *key, Color value);

#pragma endregion

#endif //KVLIST_H
