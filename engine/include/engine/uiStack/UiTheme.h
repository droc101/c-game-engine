//
// Created by droc101 on 9/29/26.
//

/**
 * @file UiTheme.h
 * Simple GUI color theme system
 */

#ifndef GAME_UITHEME_H
#define GAME_UITHEME_H

#include <engine/structs/Color.h>

/// Pair of UI colors for text
typedef struct UiTextColorset
{
	/// Foreground text color
	Color textColor;
	/// Text shadow color
	Color shadowColor;
} UiTextColorset;

/// UiStack color theme
typedef struct UiTheme
{
	/// Primary text color
	UiTextColorset primaryText;
	/// Secondary text color
	UiTextColorset secondaryText;
	/// Disabled text color
	UiTextColorset disabledText;
	/// Placeholder text color
	UiTextColorset placeholderText;
	/// Input text color
	UiTextColorset inputText;
	/// Button text color
	UiTextColorset buttonText;
	/// Slider text color
	UiTextColorset sliderText;
} UiTheme;

/// The current UI theme
extern UiTheme uiTheme;

/// @private
void LoadUiTheme();

#endif //GAME_UITHEME_H
