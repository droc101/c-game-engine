//
// Created by droc101 on 1/18/25.
//

#include "CommandParser.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../GameStates/GLoadingState.h"
#include "../Structs/GlobalState.h"
#include "Core/List.h"

void ExecuteCommand(const char *command)
{
	// need to strdup because strtok_r modifies the string (removing results in sigsegv)
	// no, passing it as non-const does not work
	char *rwCommand = strdup(command);
	List commandList;
	ListInit(commandList);
	const char *token = strtok(rwCommand, " ");
	while (token != NULL)
	{
		ListAdd(commandList, strdup(token));
		token = strtok(NULL, " ");
	}
	free(rwCommand);

	char *commandName = ListGetPointer(commandList, 0);
	if (strcmp(commandName, "level") == 0)
	{
		if (commandList.length < 2)
		{
			printf("level command requires a level name\n");
		} else
		{
			GLoadingStateSet(ListGetPointer(commandList, 1));
		}
	}
	// TODO: signal system and related commands
	else
	{
		printf("Unknown command: %s\n", commandName);
	}

	ListAndContentsFree(commandList);
}
