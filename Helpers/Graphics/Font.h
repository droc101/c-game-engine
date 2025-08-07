//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_FONT_H
#define GAME_FONT_H

#include "../../defines.h"

typedef enum FontHorizontalAlign FontHorizontalAlign;
typedef enum FontVerticalAlign FontVerticalAlign;

enum FontHorizontalAlign
{
	FONT_HALIGN_LEFT,
	FONT_HALIGN_CENTER,
	FONT_HALIGN_RIGHT
};

enum FontVerticalAlign
{
	FONT_VALIGN_TOP,
	FONT_VALIGN_MIDDLE,
	FONT_VALIGN_BOTTOM
};

/**
 * Draw a string of text to the screen
 * @param pos Top left position of the text
 * @param str String to draw
 * @param size Size of the font
 * @param color Color of the font
 * @param font The font to use
 * @return Bottom right position of the text
 * @note This is a wrapper for DrawTextAligned with h_align = FONT_HALIGN_LEFT and v_align = FONT_VALIGN_TOP
 */
void FontDrawString(Vector2 pos, const char *str, uint size, Color color, const Font *font);

/**
 * Count the number of lines in a string
 * @param str String to count lines of
 * @return Line count
 */
int StringLineCount(const char *str);

/**
 * Measure the size of a string of text
 * @param str String to measure
 * @param size Size of the font
 * @param font The font to use
 * @return Size of the text
 * @note This is a wrapper for @c MeasureTextNChars where @c n is @code strlen(str) @endcode
 */
Vector2 MeasureText(const char *str, uint size, const Font *font);

/**
 * Measure the size of a string of text up to a certain number of characters
 * @param str The string to measure
 * @param size The size of the font
 * @param font The font to use
 * @param n The number of characters to measure
 * @return The size of the text up to n characters
 * @note It is up to the caller to ensure @c n is within the bounds of @c str
 */
Vector2 MeasureTextNChars(const char *str, uint size, const Font *font, size_t n);

/**
 * Get a line of text from a string
 * @param str String to get line from
 * @param line Line number to get
 * @param out buffer to store the line in
 * @param outBufferSize Size of the buffer, if the line is longer than this it will be truncated
 * @param convertToUppercase
 * @param outBufferSize
 */
void TextGetLine(const char *str, int line, char *out, size_t outBufferSize, bool convertToUppercase);

/**
 * Draw a string of text to the screen with alignment
 * @param str String to draw
 * @param size Font size
 * @param color Font color
 * @param rectPos Top-left position of the rectangle
 * @param rectSize Size of the rectangle
 * @param hAlign Horizontal alignment of text within the rectangle
 * @param vAlign Vertical alignment of text within the rectangle
 * @param font The font to use
 */
void DrawTextAligned(const char *str,
					 uint size,
					 Color color,
					 Vector2 rectPos,
					 Vector2 rectSize,
					 FontHorizontalAlign hAlign,
					 FontVerticalAlign vAlign,
					 const Font *font);

#endif //GAME_FONT_H
