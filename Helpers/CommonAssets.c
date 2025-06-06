//
// Created by droc101 on 7/7/2024.
//

#include "CommonAssets.h"
#include "Core/AssetReader.h"
#include "Graphics/RenderingHelpers.h"

ModelDefinition *skyModel;

Font *smallFont;
Font *largeFont;

void InitCommonAssets()
{
	SetTexParams(TEXTURE("interface_menu_bg_tile"), true, true);
	SetTexParams(TEXTURE("interface_menu_bg_tile_red"), true, true);
	SetTexParams(TEXTURE("interface_font"), false, false);
	SetTexParams(TEXTURE("interface_small_fonts"), false, false);
	SetTexParams(TEXTURE("interface_button"), true, false);
	SetTexParams(TEXTURE("interface_button_hover"), true, false);
	SetTexParams(TEXTURE("interface_button_press"), true, false);
	SetTexParams(TEXTURE("interface_slider"), true, false);
	SetTexParams(TEXTURE("interface_slider_thumb"), true, false);
	SetTexParams(TEXTURE("interface_textbox"), true, false);
	SetTexParams(TEXTURE("interface_checkbox_checked"), true, false);
	SetTexParams(TEXTURE("interface_checkbox_unchecked"), true, false);
	SetTexParams(TEXTURE("interface_radio_checked"), true, false);
	SetTexParams(TEXTURE("interface_radio_unchecked"), true, false);
	SetTexParams(TEXTURE("interface_focus_rect"), true, false);
	SetTexParams(TEXTURE("level_sky"), true, true);
	SetTexParams(TEXTURE("vfx_shadow"), false, false);

	skyModel = LoadModel(MODEL("model_sky"));

	smallFont = LoadFont(FONT("font_small"));
	largeFont = LoadFont(FONT("font_large"));
}

void DestroyCommonAssets()
{
	free(smallFont);
	free(largeFont);
}
