// Fill out your copyright notice in the Description page of Project Settings.

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
