// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SSVoiceLocalizationSettings.generated.h"

/**
 * 
 */
UCLASS(config=Game, defaultconfig)
class SSVOICELOCALIZATION_API USSVoiceLocalizationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

	
public:
	USSVoiceLocalizationSettings(const FObjectInitializer& Initializer);

#if WITH_EDITOR
	virtual FText GetSectionText() const override;
#endif

public:
	
	static const USSVoiceLocalizationSettings* GetSetting();
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Voice Localization|General")
	FString DefaultLanguage = TEXT("en");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Voice Localization|General")
	FString PreviewLanguage = TEXT("en");
};
