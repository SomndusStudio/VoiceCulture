/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "SSVoiceLocalizationStyle.h"

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr<FSlateStyleSet> FSSVoiceLocalizationStyle::StyleInstance = NULL;

void FSSVoiceLocalizationStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FSSVoiceLocalizationStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FSSVoiceLocalizationStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("SSVoiceLocalizationStyle"));
	return StyleSetName;
}

TSharedPtr<ISlateStyle> FSSVoiceLocalizationStyle::Get()
{
	return StyleInstance;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )


TSharedRef<FSlateStyleSet> FSSVoiceLocalizationStyle::Create()
{
	const FVector2D Icon16x16(16.0f, 16.0f);
	const FVector2D Icon20x20(20.0f, 20.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);
	const FVector2D Icon64x64(64.0f, 64.0f);
	const FVector2D Icon128x128(128.0f, 128.0f);

	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("SSVoiceLocalization")->GetBaseDir() / TEXT("Resources"));

	// Asset icon/thumbnail
	Style->Set("ClassIcon.SSLocalizedVoiceSound", new IMAGE_BRUSH(TEXT("SSLocalizedVoiceSound128"), Icon20x20));
	Style->Set("ClassThumbnail.SSLocalizedVoiceSound", new IMAGE_BRUSH(TEXT("SSLocalizedVoiceSound128"), Icon128x128));

	Style->Set("SSLocalizedVoiceSoundEditor.Play", new IMAGE_BRUSH(TEXT("icon_Play_40x"), Icon40x40));
	Style->Set("SSLocalizedVoiceSoundEditor.Play.Small", new IMAGE_BRUSH(TEXT("icon_Play_20x"), Icon20x20));
	
	Style->Set("SSLocalizedVoiceSoundEditor.Stop", new IMAGE_BRUSH(TEXT("icon_Stop_40x"), Icon40x40));
	Style->Set("SSLocalizedVoiceSoundEditor.Stop.Small", new IMAGE_BRUSH(TEXT("icon_Stop_20x"), Icon20x20));
	
	Style->Set("SSLocalizedVoiceSoundEditor.AutoFill", new IMAGE_BRUSH(TEXT("icon_AutoFill_40x"), Icon40x40));
	Style->Set("SSLocalizedVoiceSoundEditor.AutoFill.Small", new IMAGE_BRUSH(TEXT("icon_AutoFill_20x"), Icon20x20));

	return Style;
}
