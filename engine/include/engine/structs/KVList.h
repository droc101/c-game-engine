//
// Created by droc101 on 5/28/25.
//

/**
 * @file KVList.h
 * @c KvList and @c Param definitions, macros, and functions
 */

#ifndef KVLIST_H
#define KVLIST_H

#include <engine/assets/DataReader.h>
#include <engine/assets/DataWriter.h>
#include <engine/structs/Color.h>
#include <engine/structs/Dict.h>
#include <engine/structs/Vector2.h>
#include <joltc/Math/Vector3.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#pragma region Param

/// Create a param containing a byte
#define PARAM_BYTE(byteVal) ((Param){.type = PARAM_TYPE_BYTE, .byteValue = (byteVal)})
/// Create a param containing an integer
#define PARAM_INT(intVal) ((Param){.type = PARAM_TYPE_INTEGER, .intValue = (intVal)})
/// Create a param containing a float
#define PARAM_FLOAT(floatVal) ((Param){.type = PARAM_TYPE_FLOAT, .floatValue = (floatVal)})
/// Create a param containing a boolean
#define PARAM_BOOL(boolVal) ((Param){.type = PARAM_TYPE_BOOL, .boolValue = (boolVal)})
/// Create a param containing a string
#define PARAM_STRING(stringVal) ((Param){.type = PARAM_TYPE_STRING, .stringValue = (stringVal)})
/// Create a param containing a color
#define PARAM_COLOR(colorVal) ((Param){.type = PARAM_TYPE_COLOR, .colorValue = (colorVal)})
/// Create a param containing an array of params
#define PARAM_ARRAY(arraySize, arrayData) ((Param){.type = PARAM_TYPE_ARRAY, .arrayValue.length = (arraySize), .arrayValue.data = (arrayData)})
/// Create a param containing a KvList
#define PARAM_KV_LIST(kvListVal) ((Param){.type = PARAM_TYPE_KV_LIST, .kvListValue = (kvListVal)})
/// Create a param containing a uint64_t
#define PARAM_UINT_64(uint64Val) ((Param){.type = PARAM_TYPE_UINT_64, .uint64value = (uint64Val)})
/// Create a param containing a Vector2
#define PARAM_VEC2(vec2Val) ((Param){.type = PARAM_TYPE_VEC2, .vec2value = (vec2Val)})
/// Create a param containing a Vector3
#define PARAM_VEC3(vec3Val) ((Param){.type = PARAM_TYPE_VEC3, .vec3value = (vec3Val)})
/// Create an empty param
#define PARAM_NONE ((Param){.type = PARAM_TYPE_NONE})

typedef struct Param Param;
typedef struct ParamArray ParamArray;

/// Types that can be contained within an @c Param
typedef enum ParamType
{
	/// Unsigned 8-bit integer
	PARAM_TYPE_BYTE,
	/// Signed 32-bit integer
	PARAM_TYPE_INTEGER,
	/// Single precision float
	PARAM_TYPE_FLOAT,
	/// Boolean
	PARAM_TYPE_BOOL,
	/// String
	PARAM_TYPE_STRING,
	/// Nothing
	PARAM_TYPE_NONE,
	/// Color
	PARAM_TYPE_COLOR,
	/// KvList
	PARAM_TYPE_KV_LIST,
	/// Param array
	PARAM_TYPE_ARRAY,
	/// Unsigned 64-bit integer
	PARAM_TYPE_UINT_64,
	/// Vector2
	PARAM_TYPE_VEC2,
	/// Vector3
	PARAM_TYPE_VEC3,
} ParamType;

/// Array of Params
struct ParamArray
{
	/// Length of array
	size_t length;
	/// Array data
	Param *data;
};

/// A multi-type container
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
		ParamArray arrayValue;
		uint64_t uint64value;
		Vector2 vec2value;
		Vector3 vec3value;
	};
};

#define PARAM_OPLIST_ZERO(param) \
	memset(&(param), 0, sizeof(param)); \
	(param).type = PARAM_TYPE_NONE;
#define PARAM_OPLIST_COPY(param, value) CopyParam(&(value), &(param));
#define PARAM_OPLIST_FREE(param) \
	FreeParam(&(param)); \
	PARAM_OPLIST_ZERO(param);

#define PARAM_OPLIST (INIT(PARAM_OPLIST_ZERO), INIT_SET(PARAM_OPLIST_COPY), SET(PARAM_OPLIST_COPY), CLEAR(PARAM_OPLIST_FREE))

/**
 * Read a param from bytes
 * @param reader DataReader to read from
 * @param out The param to read to
 * @param bytesRemaining Bytes remaining in DataReader buffer, must not be NULL.
 * @return Success/fail
 */
bool ReadParam(DataReader *reader, Param *out, size_t *bytesRemaining);

/**
 * Write a param to a DataWriter
 * @param param The param to write
 * @param writer The DataWriter to write to
 */
void WriteParam(const Param *param, DataWriter *writer);

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
/// @private
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
 * @param reader DataReader to read from
 * @param out The KvList to read to (will be created)
 * @param bytesRemaining Bytes remaining in DataReader buffer, must not be NULL.
 * @return Success/failure
 */
bool ReadKvList(DataReader *reader, KvList out, size_t *bytesRemaining);

/**
 * Write a KvList to a DataWriter
 * @param list The KvList to write
 * @param writer The DataWriter to write to
 */
void WriteKvList(const KvList list, DataWriter *writer);

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

/**
 * Get a uint64_t value from the key-value list.
 * @param list The list to get the value from.
 * @param key The key to get the value for.
 * @param defaultValue The default value to return if the key does not exist.
 * @return The uint64_t value associated with the key, or the default value if the key does not exist.
 */
uint64_t KvGetUint64(const KvList list, const char *key, uint64_t defaultValue);

/**
 * Get a ParamArray value from the key-value list
 * @param list The list to get the value from
 * @param key The key to get the value for
 * @return The ParamArray associated with the key, or NULL if it doesn't exist.
 */
ParamArray *KvGetArray(const KvList list, const char *key);

/**
 * Get a KvList value from the key-value list
 * @param list The list to get the value from
 * @param key The key to get the value for
 * @param out Where to put the output KvList
 * @return Success/failure
 */
bool KvGetList(const KvList list, const char *key, KvList out);

/**
 * Get a Vector2 value from the key-value list.
 * @param list The list to get the value from.
 * @param key The key to get the value for.
 * @param defaultValue The default value to return if the key does not exist.
 * @return The Vector2 value associated with the key, or the default value if the key does not exist.
 */
Vector2 KvGetVec2(const KvList list, const char *key, Vector2 defaultValue);

/**
 * Get a Vector3 value from the key-value list.
 * @param list The list to get the value from.
 * @param key The key to get the value for.
 * @param defaultValue The default value to return if the key does not exist.
 * @return The Vector3 value associated with the key, or the default value if the key does not exist.
 */
Vector3 KvGetVec3(const KvList list, const char *key, Vector3 defaultValue);

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

/**
 * Set a uint64_t value in the key-value list.
 * @param list The list to set the value in.
 * @param key The key to set the value for.
 * @param value The uint64_t value to set.
 */
void KvSetUint64(KvList list, const char *key, uint64_t value);

/**
 * Set a ParamArray value in the key-value list.
 * @param list The list to set the value in.
 * @param key The key to set the value for.
 * @param array The ParamArray value to set.
 */
void KvSetParamArray(KvList list, const char *key, ParamArray array);

/**
 * Set a KvList value in the key-value list
 * @param list The list to set the value in
 * @param key The key to set the value for
 * @param value The KvList value to set
 */
void KvSetList(KvList list, const char *key, KvList value);

/**
 * Set a Vector2 value in the key-value list.
 * @param list The list to set the value in.
 * @param key The key to set the value for.
 * @param value The Vector2 value to set.
 */
void KvSetVec2(KvList list, const char *key, Vector2 value);

/**
 * Set a Vector3 value in the key-value list.
 * @param list The list to set the value in.
 * @param key The key to set the value for.
 * @param value The Vector3 value to set.
 */
void KvSetVec3(KvList list, const char *key, Vector3 value);

/**
 * Perform a type-unsafe set of a value in the key-value list.
 * @param list The list to set the value in.
 * @param key The key to set the value for.
 * @param value The raw value to set.
 */
void KvSetUnsafe(KvList list, const char *key, Param value);

#pragma endregion

#endif //KVLIST_H
