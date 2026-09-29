//
// Created by droc101 on 9/29/26.
//

#ifndef GAME_UITHEME_H
#define GAME_UITHEME_H

#include <engine/structs/Color.h>

typedef struct UiTextColorset
{
	Color textColor;
	Color shadowColor;
} UiTextColorset;

typedef struct UiTheme
{
	UiTextColorset primaryText;
	UiTextColorset secondaryText;
	UiTextColorset disabledText;
	UiTextColorset placeholderText;
	UiTextColorset inputText;
	UiTextColorset buttonText;
	UiTextColorset sliderText;
} UiTheme;

extern UiTheme uiTheme;

void LoadUiTheme();

#endif //GAME_UITHEME_H
