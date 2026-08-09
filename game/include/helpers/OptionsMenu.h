//
// Created by droc101 on 8/7/26.
//

#ifndef GAME_OPTIONSMENUBUILDER_H
#define GAME_OPTIONSMENUBUILDER_H

#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/ScrollView.h>
#include <engine/uiStack/UiStack.h>
#include <stdbool.h>

typedef struct OptionsMenu
{
	UiStack *stack;
	ScrollView *scrollView;
	int yPos;
	bool smallOptionRightSide;
	bool locked;
} OptionsMenu;

OptionsMenu *CreateOptionsMenu();


void OptionsMenuAddSection(OptionsMenu *menu, char *label);

void OptionsMenuAddLargeControl(OptionsMenu *menu, Control *control);

void OptionsMenuAddSmallControl(OptionsMenu *menu, Control *control);


void OptionsMenuAddControl(OptionsMenu *menu, Control *control);

void OptionsMenuNextRow(OptionsMenu *menu);


void OptionsMenuAddSimpleHeaderFooter(OptionsMenu *menu, char *title, ButtonCallback doneButtonCallback);

void OptionsMenuAddOneButtonHeaderFooter(OptionsMenu *menu,
										 char *title,
										 char *buttonText,
										 ButtonCallback buttonCallback);

void OptionsMenuAddTwoButtonHeaderFooter(OptionsMenu *menu,
										 char *title,
										 char *leftButtonText,
										 ButtonCallback leftButtonCallback,
										 char *rightButtonText,
										 ButtonCallback rightButtonCallback);


void ProcessOptionsMenu(OptionsMenu *menu);

void DestroyOptionsMenu(OptionsMenu *menu);

#endif //GAME_OPTIONSMENUBUILDER_H
