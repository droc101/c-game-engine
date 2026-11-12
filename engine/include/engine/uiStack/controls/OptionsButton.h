//
// Created by droc101 on 7/31/26.
//

/**
 * @file OptionsButton.h
 * Button that cycles through various options when clicked
 */

#ifndef GAME_OPTIONSBUTTON_H
#define GAME_OPTIONSBUTTON_H

#include <engine/structs/Vector2.h>
#include <engine/uiStack/UiStack.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct OptionsButtonData OptionsButtonData;
typedef struct OptionsButtonValue OptionsButtonValue;
typedef struct OptionsButtonOptions OptionsButtonOptions;

/**
 * Callback signature for when an OptionsButton is clicked
 * @param value The current value
 * @param extraData User-provided extra data
 */
typedef void (*OptionsButtonCallback)(const OptionsButtonValue *value, void *extraData);

/// The options an OptionsButton can cycle through
struct OptionsButtonOptions
{
	/// The number of options
	size_t numValues;
	/// The options to cycle through
	OptionsButtonValue *values;
};

/// An option an OptionButton can have
struct OptionsButtonValue
{
	/// The name of this option
	char *text;
	/// The tooltip to show with this option, if allowed by the OptionButton
	char *tooltip;
	/// The value of this option
	LiteralControlValue value;
};

/// @private
struct OptionsButtonData
{
	char *text;
	char *alwaysTooltip;
	OptionsButtonCallback callback;
	bool enabled;
	OptionsButtonOptions *options;
	ControlValue value;
	void *callbackExtraData;
};

/**
 * Create a new options button control
 * @param position The control position
 * @param size The control size
 * @param text The text of the option button, including %s format specifier for option value
 * @param callback The function to call when the value changes
 * @param anchor The positioning anchor
 * @param options The options to cycle through
 * @param extraData Extra data to pass along with the callback
 * @param value The current value
 * @param alwaysTooltip The tooltip to always show, or NULL to use the tooltip from the selected option
 */
Control *CreateOptionsButtonControl(Vector2 position,
									Vector2 size,
									char *text,
									OptionsButtonCallback callback,
									ControlAnchor anchor,
									OptionsButtonOptions *options,
									void *extraData,
									ControlValue value,
									char *alwaysTooltip);

/// @private
void DestroyOptionsButton(const Control *c);

/// @private
void UpdateOptionsButton(UiStack *stack, Control *c, Vector2 localMousePos, uint32_t ctlIndex);

/// @private
void DrawOptionsButton(const Control *c, ControlState state, Vector2 position);

/// On/off boolean options
extern OptionsButtonOptions onOffButtonOptions;
/// Yes/no boolean options
extern OptionsButtonOptions yesNoButtonOptions;

#endif //GAME_OPTIONSBUTTON_H
