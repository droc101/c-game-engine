//
// Created by droc101 on 7/19/26.
//

#include <engine/debug/DebugGraph.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/helpers/MathEx.h>
#include <engine/structs/Color.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Timing.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define NANOSECONDS_PER_SECOND 1000000000.0

static const Color COLOR_GREAT = COLOR(0xff00ffff);
static const Color COLOR_GOOD = COLOR(0xff00ff00);
static const Color COLOR_INTERMEDIATE = COLOR(0xffff8000);
static const Color COLOR_BAD = COLOR(0xffff0000);

DebugGraph *CreateDebugGraph(const size_t numDataPoints,
							 const uint64_t updateIntervalMsec,
							 const double goodThreshold,
							 const double badThreshold,
							 const double maxShownValue,
							 char *label,
							 char *linearLabel)
{
	assert(goodThreshold > badThreshold);
	assert(maxShownValue >= goodThreshold);

	DebugGraph *graph = malloc(sizeof(DebugGraph));
	CheckAlloc(graph);
	graph->goodThreshold = goodThreshold;
	graph->maxShownValue = maxShownValue;
	graph->badThreshold = badThreshold;
	graph->nspf = NANOSECONDS_PER_SECOND / goodThreshold;
	graph->numDataPoints = numDataPoints;
	graph->updateIntervalMsec = updateIntervalMsec;
	graph->label = strdup(label);
	CheckAlloc(graph->label);
	graph->linearLabel = strdup(linearLabel);
	CheckAlloc(graph->linearLabel);
	graph->data = calloc(numDataPoints, sizeof(double));
	CheckAlloc(graph->data);
	return graph;
}

void DestroyDebugGraph(DebugGraph *graph)
{
	free(graph->data);
	free(graph->label);
	free(graph->linearLabel);
	free(graph);
}

void DebugGraphPush(DebugGraph *graph, const uint64_t ns)
{
	// If it's not time to update the graph, return
	if (GetTimeMs() - graph->lastUpdateTimeMsec < graph->updateIntervalMsec)
	{
		return;
	}

	const double value = ns == 0 ? 1 : (double)ns;

	for (size_t i = 0; i < graph->numDataPoints - 1; i++)
	{
		graph->data[i] = graph->data[i + 1];
	}
	graph->data[graph->numDataPoints - 1] = value;

	graph->lastUpdateTimeMsec = GetTimeMs();
}

static float DebugGraphValueHeight(DebugGraph *graph, const Vector2 pos, const Vector2 size, const double value)
{
	const float availHeight = size.y - 20.0f;
	return (float)(pos.y + size.y - 10 - remap(value, 0, graph->maxShownValue, 0, availHeight));
}

static float DebugGraphSampleX(DebugGraph *graph, const Vector2 pos, const Vector2 size, const size_t sample)
{
	const float availWidth = size.x - 20;
	return pos.x + size.x - 10 - remap(sample, 0, graph->numDataPoints, availWidth, 0);
}

static void DebugGraphYLabel(DebugGraph *graph, Vector2 pos, Vector2 size, double value, Color color)
{
	char label[40];
	sprintf(label, "%.0f %s", value, graph->label);

	color.a = 0.5f;

	const float targetLineY = DebugGraphValueHeight(graph, pos, size, value);
	DrawLine(v2(pos.x + 10, targetLineY), v2(pos.x + size.x - 10, targetLineY), 2, COLOR(0x80808080));
	FontDrawString(v2(pos.x + 10, targetLineY + 2), label, 12, color, smallFont);
}

void DrawDebugGraph(DebugGraph *graph, const Vector2 pos, const Vector2 size)
{
	DrawRect((int)pos.x, (int)pos.y, (int)size.x, (int)size.y, COLOR(0x80000000));
	DrawOutlineRect(pos, size, 1, COLOR(0x80808080));

	DrawLine(v2(pos.x + 10, pos.y + size.y - 10), v2(pos.x + size.x - 10, pos.y + size.y - 10), 2, COLOR(0x80808080));

	Color lineColor = COLOR(0);
	for (int64_t i = (int64_t)graph->numDataPoints - 1; i >= 0; i--)
	{
		if (graph->data[i] == 0)
		{
			break;
		}

		const double ns = graph->data[i];
		const double nextNs = graph->data[i + (i == (int64_t)graph->numDataPoints - 1 ? 0 : 1)];

		double value = NANOSECONDS_PER_SECOND / ns;
		double nextValue = NANOSECONDS_PER_SECOND / nextNs;

		double nsRemapped = remap(ns, 0, graph->nspf, 0, graph->goodThreshold);
		double nextNsRemapped = remap(nextNs, 0, graph->nspf, 0, graph->goodThreshold);

		if (value > graph->maxShownValue)
		{
			value = graph->maxShownValue;
		}
		if (nextValue > graph->maxShownValue)
		{
			nextValue = graph->maxShownValue;
		}
		if (nsRemapped > graph->maxShownValue)
		{
			nsRemapped = graph->maxShownValue;
		}
		if (nextNsRemapped > graph->maxShownValue)
		{
			nextNsRemapped = graph->maxShownValue;
		}

		const double x1 = DebugGraphSampleX(graph, pos, size, i);
		double y1 = DebugGraphValueHeight(graph, pos, size, value);
		const double x2 = DebugGraphSampleX(graph, pos, size, i + 1);
		double y2 = DebugGraphValueHeight(graph, pos, size, nextValue);

		if (round(value) >= graph->goodThreshold)
		{
			lineColor = COLOR_GOOD;
		} else if (value < graph->badThreshold)
		{
			lineColor = COLOR_BAD;
		} else
		{
			lineColor = COLOR_INTERMEDIATE;
		}
		DrawLine(v2((float)x1, (float)y1), v2((float)x2, (float)y2), 2, lineColor);

		y1 = DebugGraphValueHeight(graph, pos, size, nsRemapped);
		y2 = DebugGraphValueHeight(graph, pos, size, nextNsRemapped);
		lineColor.a = 0.5f;
		DrawLine(v2((float)x1, (float)y1), v2((float)x2, (float)y2), 2, lineColor);
	}

	DebugGraphYLabel(graph, pos, size, graph->goodThreshold, COLOR_GOOD);
	if (graph->maxShownValue != graph->goodThreshold)
	{
		DebugGraphYLabel(graph, pos, size, graph->maxShownValue, COLOR_GREAT);
	}
	DebugGraphYLabel(graph, pos, size, graph->badThreshold, COLOR_BAD);

	const double currentNs = graph->data[graph->numDataPoints - 1];
	const double currentValue = 1000000000.0 / currentNs;
	const int currentValueInt = (int)round(currentValue);
	const double currentMs = currentNs / 1000000.0;

	if (currentValueInt >= graph->goodThreshold)
	{
		lineColor = COLOR_GOOD;
	} else if (currentValueInt < graph->badThreshold)
	{
		lineColor = COLOR_BAD;
	} else
	{
		lineColor = COLOR_INTERMEDIATE;
	}

	const size_t fontSize = 16;
	const float yPos = pos.y + size.y - 10 - (float)fontSize - 6;

	char label[40];
	sprintf(label, "%s: %.2f", graph->label, currentValue);
	float xPos = pos.x + 10;
	FontDrawString(v2(xPos + 2, yPos + 2), label, fontSize, COLOR_BLACK, smallFont);
	FontDrawString(v2(xPos, yPos), label, fontSize, lineColor, smallFont);

	char linearLabel[40];
	sprintf(linearLabel, "%.2f %s", currentMs, graph->linearLabel);
	const float labelSize = MeasureText(linearLabel, fontSize, smallFont).x;
	xPos = pos.x + size.x - 10 - labelSize;
	FontDrawString(v2(xPos + 2, yPos + 2), linearLabel, fontSize, COLOR_BLACK, smallFont);
	FontDrawString(v2(xPos, yPos), linearLabel, fontSize, lineColor, smallFont);
}

void DrawDebugGraphText(DebugGraph *graph, Vector2 pos, Vector2 size, bool alignRight)
{
	Color lineColor = COLOR(0);
	const double currentNs = graph->data[graph->numDataPoints - 1];
	const double currentValue = 1000000000.0 / currentNs;
	const int currentValueInt = (int)round(currentValue);
	const double currentMs = currentNs / 1000000.0;

	if (currentValueInt >= graph->goodThreshold)
	{
		lineColor = COLOR_GOOD;
	} else if (currentValueInt < graph->badThreshold)
	{
		lineColor = COLOR_BAD;
	} else
	{
		lineColor = COLOR_INTERMEDIATE;
	}

	const size_t fontSize = 16;
	const float yPos = pos.y + size.y - 10 - (float)fontSize * 2 - 6;

	char label[40];
	sprintf(label, "%s: %.2f\n%s: %.2f", graph->label, currentValue, graph->linearLabel, currentMs);
	float xPos = pos.x + 10;
	if (alignRight)
	{
		const float labelSize = MeasureText(label, fontSize, smallFont).x;
		xPos = pos.x + size.x - 10 - labelSize;
	}
	FontDrawString(v2(xPos + 2, yPos + 2), label, fontSize, COLOR_BLACK, smallFont);
	FontDrawString(v2(xPos, yPos), label, fontSize, lineColor, smallFont);
}
