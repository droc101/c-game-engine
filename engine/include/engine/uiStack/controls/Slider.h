//
// Created by droc101 on 10/7/2024.
//

#ifndef GAME_SLIDER_H
#define GAME_SLIDER_H

#include <stdint.h>
#include <engine/structs/Vector2.h>
#include <engine/uiStack/UiStack.h>

typedef struct SliderData SliderData;

typedef void (*SliderCallback)(float value);
typedef char *(*SliderLabelFunction)(const Control *slider);

struct SliderData
{
	char *label;

	double min;
	double max;
	double value;

	double step;
	double altStep; // Step when holding shift

	SliderCallback callback;
	SliderLabelFunction getLabel;
};

char *SliderLabelPercent(const Control *slider);

char *SliderLabelInteger(const Control *slider);

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
 * @return The new Slider Control
 */
Control *CreateSliderControl(Vector2 position,
							 Vector2 size,
							 char *label,
							 SliderCallback callback,
							 ControlAnchor anchor,
							 double min,
							 double max,
							 double value,
							 double step,
							 double altStep,
							 SliderLabelFunction getLabel);

void DestroySlider(const Control *c);

void UpdateSlider(UiStack *stack, Control *c, Vector2 localMousePos, uint32_t ctlIndex);

void DrawSlider(const Control *c, ControlState state, Vector2 position);

#endif //GAME_SLIDER_H
