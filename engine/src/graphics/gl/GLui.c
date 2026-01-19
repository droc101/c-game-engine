//
// Created by droc101 on 11/20/25.
//

#include <cglm/cglm.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/gl/GLframe.h>
#include <engine/graphics/gl/GLobjects.h>
#include <engine/graphics/gl/GLshaders.h>
#include <engine/graphics/gl/GLui.h>
#include <engine/structs/Color.h>
#include <engine/structs/Vector2.h>
#include <stddef.h>
#include <stdint.h>

void GL_DrawRect(const Vector2 pos, const Vector2 size, const Color color)
{
	GL_UseShader(uiColoredShader);

	glUniform4fv(uiColoredColorLoc, 1, COLOR_TO_ARR(color));

	const Vector2 ndcStartPos = v2(GL_X_TO_NDC(pos.x), GL_Y_TO_NDC(pos.y));
	const Vector2 ncdEndPos = v2(GL_X_TO_NDC(pos.x + size.x), GL_Y_TO_NDC(pos.y + size.y));

	const float vertices[4][2] = {
		{(float)ndcStartPos.x, (float)ndcStartPos.y},
		{(float)ncdEndPos.x, (float)ndcStartPos.y},
		{(float)ncdEndPos.x, (float)ncdEndPos.y},
		{(float)ndcStartPos.x, (float)ncdEndPos.y},
	};

	const uint32_t indices[] = {0, 2, 1, 0, 3, 2};

	GL_BindBuffer(glBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW);

	glVertexAttribPointer(uiColoredVertexLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(uiColoredVertexLoc);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void GL_DrawRectOutline(const Vector2 pos, const Vector2 size, const Color color, const float thickness)
{
	if (thickness < 1.0f)
	{
		glEnable(GL_LINE_SMOOTH);
	} else
	{
		glDisable(GL_LINE_SMOOTH);
	}

	glLineWidth(thickness);

	GL_UseShader(uiColoredShader);

	glUniform4fv(uiColoredColorLoc, 1, COLOR_TO_ARR(color));

	const Vector2 ndcStartPos = v2(GL_X_TO_NDC(pos.x), GL_Y_TO_NDC(pos.y));
	const Vector2 ndcEndPos = v2(GL_X_TO_NDC(pos.x + size.x), GL_Y_TO_NDC(pos.y + size.y));


	const float vertices[4][2] = {
		{(float)ndcStartPos.x, (float)ndcStartPos.y},
		{(float)ndcEndPos.x, (float)ndcStartPos.y},
		{(float)ndcEndPos.x, (float)ndcEndPos.y},
		{(float)ndcStartPos.x, (float)ndcEndPos.y},
	};

	const uint32_t indices[] = {0, 1, 2, 3};

	GL_BindBuffer(glBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW);

	glVertexAttribPointer(uiColoredVertexLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(uiColoredVertexLoc);

	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, NULL);
}

void GL_DrawTexture_Internal(const Vector2 pos,
							 const Vector2 size,
							 const char *texture,
							 const Color color,
							 const Vector2 regionStart,
							 const Vector2 regionEnd)
{
	GL_UseShader(uiTexturedShader);

	GL_LoadTextureFromAsset(texture);

	glUniform4fv(uiTexturedColorLoc, 1, COLOR_TO_ARR(color));

	glUniform4f(uiTexturedRegionLoc,
				(GLfloat)regionStart.x,
				(GLfloat)regionStart.y,
				(GLfloat)regionEnd.x,
				(GLfloat)regionEnd.y);

	const Vector2 ndcStartPos = v2(GL_X_TO_NDC(pos.x), GL_Y_TO_NDC(pos.y));
	const Vector2 ndcEndPos = v2(GL_X_TO_NDC(pos.x + size.x), GL_Y_TO_NDC(pos.y + size.y));


	const float vertices[4][4] = {
		{(float)ndcStartPos.x, (float)ndcStartPos.y, 0.0f, 0.0f},
		{(float)ndcEndPos.x, (float)ndcStartPos.y, 1.0f, 0.0f},
		{(float)ndcEndPos.x, (float)ndcEndPos.y, 1.0f, 1.0f},
		{(float)ndcStartPos.x, (float)ndcEndPos.y, 0.0f, 1.0f},
	};

	const uint32_t indices[] = {0, 2, 1, 0, 3, 2};

	GL_BindBuffer(glBuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW);

	glVertexAttribPointer(uiTexturedVertexLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(uiTexturedVertexLoc);

	glVertexAttribPointer(uiTexturedUvLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(uiTexturedUvLoc);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

inline void GL_DrawTexture(const Vector2 pos, const Vector2 size, const char *texture)
{
	GL_DrawTexture_Internal(pos, size, texture, COLOR_WHITE, v2(-1, 0), v2s(0));
}

inline void GL_DrawTextureMod(const Vector2 pos, const Vector2 size, const char *texture, const Color color)
{
	GL_DrawTexture_Internal(pos, size, texture, color, v2(-1, 0), v2s(0));
}

inline void GL_DrawTextureRegion(const Vector2 pos,
								 const Vector2 size,
								 const char *texture,
								 const Vector2 regionStart,
								 const Vector2 regionEnd)
{
	GL_DrawTexture_Internal(pos, size, texture, COLOR_WHITE, regionStart, regionEnd);
}

inline void GL_DrawTextureRegionMod(const Vector2 pos,
									const Vector2 size,
									const char *texture,
									const Vector2 regionStart,
									const Vector2 regionEnd,
									const Color color)
{
	GL_DrawTexture_Internal(pos, size, texture, color, regionStart, regionEnd);
}

void GL_DrawLine(const Vector2 start, const Vector2 end, const Color color, const float thickness)
{
	if (thickness < 1.0f)
	{
		glEnable(GL_LINE_SMOOTH);
	} else
	{
		glDisable(GL_LINE_SMOOTH);
	}

	GL_UseShader(uiColoredShader);

	glUniform4fv(uiColoredColorLoc, 1, COLOR_TO_ARR(color));

	const Vector2 ndcStartPos = v2(GL_X_TO_NDC(start.x), GL_Y_TO_NDC(start.y));
	const Vector2 ndcEndPos = v2(GL_X_TO_NDC(end.x), GL_Y_TO_NDC(end.y));

	// Calculate the 2 corner vertices of each point for a thick line
	const float vertices[2][2] = {
		{(float)ndcStartPos.x, (float)ndcStartPos.y},
		{(float)ndcEndPos.x, (float)ndcEndPos.y},
	};

	const uint32_t indices[] = {0, 1};

	GL_BindBuffer(glBuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW);

	glVertexAttribPointer(uiColoredVertexLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(uiColoredVertexLoc);

	glLineWidth(thickness);
	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, NULL);
}

void GL_DrawColoredArrays(const float *vertices, const uint32_t *indices, const uint32_t quadCount, const Color color)
{
	GL_UseShader(uiColoredShader);

	glUniform4fv(uiColoredColorLoc, 1, COLOR_TO_ARR(color));

	GL_BindBuffer(glBuffer);

	glBufferData(GL_ARRAY_BUFFER, (long)(quadCount * 16 * sizeof(float)), vertices, GL_STREAM_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (long)(quadCount * 6 * sizeof(uint32_t)), indices, GL_STREAM_DRAW);

	glVertexAttribPointer(uiColoredVertexLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(uiColoredVertexLoc);

	glDrawElements(GL_TRIANGLES, (int)(quadCount * 6), GL_UNSIGNED_INT, NULL);
}

void GL_DrawTexturedArrays(const float *vertices,
						   const uint32_t *indices,
						   const int quadCount,
						   const char *texture,
						   const Color color)
{
	GL_UseShader(uiTexturedShader);

	GL_LoadTextureFromAsset(texture);

	glUniform4fv(uiTexturedColorLoc, 1, COLOR_TO_ARR(color));

	glUniform4f(uiTexturedRegionLoc, -1, 0, 0, 0);

	GL_BindBuffer(glBuffer);

	glBufferData(GL_ARRAY_BUFFER, (long)(quadCount * 16 * sizeof(float)), vertices, GL_STREAM_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (long)(quadCount * 6 * sizeof(uint32_t)), indices, GL_STREAM_DRAW);

	glVertexAttribPointer(uiTexturedVertexLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(uiTexturedVertexLoc);

	glVertexAttribPointer(uiTexturedUvLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(uiTexturedUvLoc);

	glDrawElements(GL_TRIANGLES, quadCount * 6, GL_UNSIGNED_INT, NULL);
}

void GL_DrawUITriangles(const UiTriangleArray *tris, const char *texture, const Color col)
{
	GL_UseShader(uiTexturedShader);

	GL_LoadTextureFromAsset(texture);

	glUniform4fv(uiTexturedColorLoc, 1, COLOR_TO_ARR(col));

	glUniform4f(uiTexturedRegionLoc, -1, 0, 0, 0);

	GL_BindBuffer(glBuffer);

	glBufferData(GL_ARRAY_BUFFER, (long)(tris->vertexCount * 4 * sizeof(float)), tris->vertices, GL_STREAM_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (long)(tris->indexCount * sizeof(uint32_t)), tris->indices, GL_STREAM_DRAW);

	glVertexAttribPointer(uiTexturedVertexLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(uiTexturedVertexLoc);

	glVertexAttribPointer(uiTexturedUvLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(uiTexturedUvLoc);

	glDrawElements(GL_TRIANGLES, (GLsizei)tris->indexCount, GL_UNSIGNED_INT, NULL);
}
