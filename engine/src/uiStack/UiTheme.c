//
// Created by droc101 on 9/29/26.
//

#include <engine/assets/AssetReader.h>
#include <engine/assets/DataReader.h>
#include <engine/structs/KVList.h>
#include <engine/subsystem/Error.h>
#include <engine/uiStack/UiTheme.h>

UiTheme uiTheme;

void LoadUiTheme()
{
	Asset *asset = LoadAsset("ui_theme.gkvl", false, false);
	if (!asset || asset->type != ASSET_TYPE_KV_LIST || asset->typeVersion != 1)
	{
		Error("Invalid UI theme!");
	}
	DataReader *reader = CreateDataReaderFromAsset(asset);
	size_t bytesRemaining = asset->size;
	KvList themeList = {};
	if (!ReadKvList(reader, themeList, &bytesRemaining))
	{
		Error("Failed to read UI theme!");
	}
	DestroyDataReader(reader);

	uiTheme.primaryText.textColor = KvGetColor(themeList, "primary_text", COLOR_WHITE);
	uiTheme.primaryText.shadowColor = KvGetColor(themeList, "primary_text_shadow", COLOR_BLACK);

	uiTheme.secondaryText.textColor = KvGetColor(themeList, "secondary_text", COLOR_WHITE);
	uiTheme.secondaryText.shadowColor = KvGetColor(themeList, "secondary_text_shadow", COLOR_BLACK);

	uiTheme.disabledText.textColor = KvGetColor(themeList, "disabled_text", COLOR_WHITE);
	uiTheme.disabledText.shadowColor = KvGetColor(themeList, "disabled_text_shadow", COLOR_BLACK);

	uiTheme.placeholderText.textColor = KvGetColor(themeList, "placeholder_text", COLOR_WHITE);
	uiTheme.placeholderText.shadowColor = KvGetColor(themeList, "placeholder_text_shadow", COLOR_BLACK);

	uiTheme.inputText.textColor = KvGetColor(themeList, "input_text", COLOR_WHITE);
	uiTheme.inputText.shadowColor = KvGetColor(themeList, "input_text_shadow", COLOR_BLACK);

	uiTheme.buttonText.textColor = KvGetColor(themeList, "button_text", COLOR_WHITE);
	uiTheme.buttonText.shadowColor = KvGetColor(themeList, "button_text_shadow", COLOR_BLACK);

	uiTheme.sliderText.textColor = KvGetColor(themeList, "slider_text", COLOR_WHITE);
	uiTheme.sliderText.shadowColor = KvGetColor(themeList, "slider_text_shadow", COLOR_BLACK);

	KvListDestroy(themeList);

	FreeAsset(asset);
}
