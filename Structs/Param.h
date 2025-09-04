//
// Created by droc101 on 9/3/25.
//

#ifndef GAME_PARAM_H
#define GAME_PARAM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "Color.h"

#define PARAM_BYTE(x) ((Param){PARAM_TYPE_BYTE, .byteValue = (x)})
#define PARAM_INT(x) ((Param){PARAM_TYPE_INTEGER, .intValue = (x)})
#define PARAM_FLOAT(x) ((Param){PARAM_TYPE_FLOAT, .floatValue = (x)})
#define PARAM_BOOL(x) ((Param){PARAM_TYPE_BOOL, .boolValue = (x)})
#define PARAM_STRING(x) ((Param){PARAM_TYPE_STRING, .stringValue = (x)})
#define PARAM_COLOR(x) ((Param){PARAM_TYPE_COLOR, .colorValue = (x)})
#define PARAM_NONE ((Param){PARAM_TYPE_NONE})

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
	PARAM_TYPE_COLOR
};

struct Param
{
	ParamType type;
	union
	{
		uint8_t byteValue;
		int intValue;
		float floatValue;
		bool boolValue;
		char stringValue[64];
		Color colorValue;
	};
};

#define PARAM_OPL_ZERO(param) \
	memset(&(param), 0, sizeof(param)); \
	(param).type = PARAM_TYPE_NONE;
#define PARAM_OPL_COPY(param, value) memcpy(&(param), &(value), sizeof(param));
#define PARAM_OPL_FREE(param) PARAM_OPL_ZERO(param);

#define PARAM_OPLIST (INIT(PARAM_OPL_ZERO), INIT_SET(PARAM_OPL_COPY), SET(PARAM_OPL_COPY), CLEAR(PARAM_OPL_FREE))

#endif //GAME_PARAM_H
