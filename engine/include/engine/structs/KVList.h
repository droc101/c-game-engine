//
// Created by droc101 on 5/28/25.
//

#ifndef KVLIST_H
#define KVLIST_H

#include <engine/structs/Color.h>
#include <engine/structs/Dict.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#pragma region Param

#define PARAM_BYTE(x) ((Param){.type = PARAM_TYPE_BYTE, .byteValue = (x)})
#define PARAM_INT(x) ((Param){.type = PARAM_TYPE_INTEGER, .intValue = (x)})
#define PARAM_FLOAT(x) ((Param){.type = PARAM_TYPE_FLOAT, .floatValue = (x)})
#define PARAM_BOOL(x) ((Param){.type = PARAM_TYPE_BOOL, .boolValue = (x)})
#define PARAM_STRING(x) ((Param){.type = PARAM_TYPE_STRING, .stringValue = (x)})
#define PARAM_COLOR(x) ((Param){.type = PARAM_TYPE_COLOR, .colorValue = (x)})
#define PARAM_ARRAY(s, d) ((Param){.type = PARAM_TYPE_ARRAY, .arrayValue.length = (s), .arrayValue.data = (d)})
#define PARAM_KV_LIST(x) ((Param){.type = PARAM_TYPE_KV_LIST, .kvListValue = (x)})
#define PARAM_UINT_64(x) ((Param){.type = PARAM_TYPE_UINT_64, .uint64value = (x)})
#define PARAM_NONE ((Param){.type = PARAM_TYPE_NONE})

typedef enum ParamType ParamType;

typedef struct Param Param;

enum ParamType
{
	PARAM_TYPE_BYTE,
	PARAM_TYPE_INTEGER,
	PARAM_TYPE_FLOAT,
	PARAM_TYPE_BOOL,
	PARAM_TYPE_STRING,
	PARAM_TYPE_NONE,
	PARAM_TYPE_COLOR,
	PARAM_TYPE_KV_LIST,
	PARAM_TYPE_ARRAY,
	PARAM_TYPE_UINT_64
};

struct Param
{
	/// The type contained in this param
	ParamType type;
	union
	{
		uint8_t byteValue;
		int intValue;
		float floatValue;
		bool boolValue;
		char *stringValue;
		Color colorValue;
		struct KvList_s *kvListValue;
		struct
		{
			size_t length;
			Param *data;
		} arrayValue;
		uint64_t uint64value;
	};
};

#define PARAM_OPL_ZERO(param) \
	memset(&(param), 0, sizeof(param)); \
	(param).type = PARAM_TYPE_NONE;
#define PARAM_OPL_COPY(param, value) CopyParam(&(value), &(param));
#define PARAM_OPL_FREE(param) \
	FreeParam(&(param)); \
	PARAM_OPL_ZERO(param);

#define PARAM_OPLIST (INIT(PARAM_OPL_ZERO), INIT_SET(PARAM_OPL_COPY), SET(PARAM_OPL_COPY), CLEAR(PARAM_OPL_FREE))

/**
 * Read a param from bytes
 * @param data Data to read from
 * @param dataSize Total size of the data
 * @param offset Offset into the data (will be updated)
 * @param out The param to read to
 * @return Number of bytes read
 */
size_t ReadParam(const void *data, size_t dataSize, size_t *offset, Param *out);

/**
 * Copy param @c source into @c dest
 */
void CopyParam(const Param *source, Param *dest);

/**
 * Free a param
 * @param param The param to free
 */
void FreeParam(Param *param);

#pragma endregion

// TODO find a way to not leak this into anyone who includes this
DEFINE_DICT(KvList, const char *, STR_OPLIST, Param, PARAM_OPLIST);

/**
 * Creates a key-value list.
 * @param list The list to create.
 */
void KvListCreate(KvList list);

/**
 * Copy KvList @c source to @c dest
 */
void KvListCopy(const KvList source, KvList dest);

/**
 * Read a KvList from bytes
 * @param data Data to read from
 * @param dataSize Total size of the data
 * @param offset Offset into the data (will be updated)
 * @param out The KvList to read to (will be created)
 * @return Number of bytes read
 */
size_t ReadKvList(const void *data, size_t dataSize, size_t *offset, KvList out);

/**
 * Destroys a key-value list, freeing all memory associated with it.
 * @param list The list to destroy.
 */
void KvListDestroy(KvList list);

/**
 * Deletes a key from a key-value list.
 * @param list The list to delete from.
 * @param key The key to delete.
 */
void KvDelete(KvList list, const char *key);

/**
 * Check if a key-value list contains a key of a given type
 * @param list The list to check
 * @param key The key to check for
 * @param expectedType The type to check for
 */
bool KvHas(KvList list, const char *key, ParamType expectedType);

#pragma region Getters

/**
 * Get a byte value from the key-value list.
 * @param list The list to get the value from.
 * @param key The key to get the value for.
 * @param defaultValue The default value to return if the key does not exist.
 * @return The byte value associated with the key, or the default value if the key does not exist.
 */
uint8_t KvGetByte(const KvList list, const char *key, uint8_t defaultValue);

/**
 * Get an integer value from the key-value list.
 * @param list The list to get the value from.
 * @param key The key to get the value for.
 * @param defaultValue The default value to return if the key does not exist.
 * @return The integer value associated with the key, or the default value if the key does not exist.
 */
int KvGetInt(const KvList list, const char *key, int defaultValue);

/**
 * Get a float value from the key-value list.
 * @param list The list to get the value from.
 * @param key The key to get the value for.
 * @param defaultValue The default value to return if the key does not exist.
 * @return The float value associated with the key, or the default value if the key does not exist.
 */
float KvGetFloat(const KvList list, const char *key, float defaultValue);

/**
 * Get a boolean value from the key-value list.
 * @param list The list to get the value from.
 * @param key The key to get the value for.
 * @param defaultValue The default value to return if the key does not exist.
 * @return The boolean value associated with the key, or the default value if the key does not exist.
 */
bool KvGetBool(const KvList list, const char *key, bool defaultValue);

/**
 * Get a string value from the key-value list.
 * @param list The list to get the value from.
 * @param key The key to get the value for.
 * @param defaultValue The default value to return if the key does not exist.
 * @return The string value associated with the key, or the default value if the key does not exist.
 */
const char *KvGetString(const KvList list, const char *key, const char *defaultValue);

/**
 * Get a Color value from the key-value list.
 * @param list The list to get the value from.
 * @param key The key to get the value for.
 * @param defaultValue The default value to return if the key does not exist.
 * @return The Color value associated with the key, or the default value if the key does not exist.
 */
Color KvGetColor(const KvList list, const char *key, Color defaultValue);

uint64_t KvGetUint64(const KvList list, const char *key, uint64_t defaultValue);

#pragma endregion

#pragma region Setters

/**
 * Set a byte value in the key-value list.
 * @param list The list to set the value in.
 * @param key The key to set the value for.
 * @param value The byte value to set.
 */
void KvSetByte(KvList list, const char *key, uint8_t value);

/**
 * Set an integer value in the key-value list.
 * @param list The list to set the value in.
 * @param key The key to set the value for.
 * @param value The integer value to set.
 */
void KvSetInt(KvList list, const char *key, int value);

/**
 * Set a float value in the key-value list.
 * @param list The list to set the value in.
 * @param key The key to set the value for.
 * @param value The float value to set.
 */
void KvSetFloat(KvList list, const char *key, float value);

/**
 * Set a boolean value in the key-value list.
 * @param list The list to set the value in.
 * @param key The key to set the value for.
 * @param value The boolean value to set.
 */
void KvSetBool(KvList list, const char *key, bool value);

/**
 * Set a string value in the key-value list.
 * @param list The list to set the value in.
 * @param key The key to set the value for.
 * @param value The string value to set.
 */
void KvSetString(KvList list, const char *key, const char *value);

/**
 * Set a Color value in the key-value list.
 * @param list The list to set the value in.
 * @param key The key to set the value for.
 * @param value The Color value to set.
 */
void KvSetColor(KvList list, const char *key, Color value);

void KvSetUint64(KvList list, const char *key, uint64_t value);

/**
 * Perform a type-unsafe set of a value in the key-value list.
 * @param list The list to set the value in.
 * @param key The key to set the value for.
 * @param value The raw value to set.
 */
void KvSetUnsafe(KvList list, const char *key, Param value);

#pragma endregion

#endif //KVLIST_H
