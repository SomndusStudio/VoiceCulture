/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"

#if ENGINE_MAJOR_VERSION >= 5
#define SS_STYLE_NAMESPACE FAppStyle
#else
	#define SS_STYLE_NAMESPACE FEditorStyle
#endif

namespace SSVoiceStyleCompat
{
	// Get the global style set (AppStyle or EditorStyle depending on engine)
	FORCEINLINE const ISlateStyle& Get()
	{
		return SS_STYLE_NAMESPACE::Get();
	}

	// Get a specific brush from the global style
	FORCEINLINE const FSlateBrush* GetBrush(FName BrushName)
	{
		return SS_STYLE_NAMESPACE::GetBrush(BrushName);
	}

	// Get a specific brush from the global style
	FORCEINLINE FName GetStyleSetName()
	{
#if ENGINE_MAJOR_VERSION >= 5
		return FAppStyle::GetAppStyleSetName();
#else
		return FEditorStyle::GetStyleSetName();
#endif
	}
}
