//
// Created by droc101 on 10/7/2024.
//

/**
 * @file Slider.h
 * Simple slider control
 */

#ifndef GAME_SLIDER_H
#define GAME_SLIDER_H

#include <engine/structs/Vector2.h>
#include <engine/uiStack/UiStack.h>
#include <stdint.h>

typedef struct SliderData SliderData;

/**
 * Callback signature for when a slider's value changes
 * @param value The slider's value
 */
typedef void (*SliderCallback)(const ControlValue *value);
/**
 * Callback signature for getting the slider's label
 * @param slider The slider to get the label for
 * @return The text to show on the slider
 */
typedef char *(*SliderLabelFunction)(const Control *slider);

/// @private
struct SliderData
{
	char *label;

	float min;
	float max;
	ControlValue value;

	float step;
	float altStep; // Step when holding shift

	SliderCallback callback;
	SliderLabelFunction getLabel;
};

/// Slider label function to show a percent
char *SliderLabelPercent(const Control *);

/// Slider label function to show an integer
char *SliderLabelInteger(const Control *);

/**
 * Get a slider's value as a float
 * @param data The slider data to get the value from
 * @return The slider's value as a float
 */
float GetSliderValueAsFloat(const SliderData *data);

/**
 * Create a new Slider Control
 * @param position The position of the slider
 * @param size The size of the slider
 * @param label The label of the slider
 * @param callback The callback function to call when the slider value changes
 * @param anchor The anchor of the slider
 * @param min The minimum value of the slider
 * @param max The maximum value of the slider
 * @param value The initial value of the slider
 * @param step The step value of the slider
 * @param altStep The step value of the slider when holding shift
 * @param getLabel The function to get the label of the slider
 * @param tooltip The tooltip to show on the slider
 * @return The new Slider Control
 */
Control *CreateSliderControl(Vector2 position,
							 Vector2 size,
							 char *label,
							 SliderCallback callback,
							 ControlAnchor anchor,
							 float min,
							 float max,
							 ControlValue value,
							 float step,
							 float altStep,
							 SliderLabelFunction getLabel,
							 char *tooltip);

/// @private
void DestroySlider(const Control *c);

/// @private
void UpdateSlider(UiStack *stack, Control *c, Vector2 localMousePos, uint32_t ctlIndex);

/// @private
void DrawSlider(const Control *c, ControlState state, Vector2 position);

#endif //GAME_SLIDER_H
