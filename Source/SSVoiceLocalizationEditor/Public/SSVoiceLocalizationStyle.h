/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"

class SSVOICELOCALIZATIONEDITOR_API FSSVoiceLocalizationStyle
{
public:
	static void Initialize();
	static void Shutdown();
	static FName GetStyleSetName();
	static TSharedPtr<class ISlateStyle> Get();

private:
	static TSharedRef<class FSlateStyleSet> Create();

private:
	static TSharedPtr<class FSlateStyleSet> StyleInstance;
};
