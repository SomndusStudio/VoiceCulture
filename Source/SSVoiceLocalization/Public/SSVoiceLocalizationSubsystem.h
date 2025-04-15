// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "SSVoiceLocalizationSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class SSVOICELOCALIZATION_API USSVoiceLocalizationSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetCurrentVoiceCulture(const FString& Language);

	UFUNCTION(BlueprintCallable)
	FString GetCurrentVoiceCulture() const;

#if WITH_EDITOR
	FString GetEditorPreviewLanguage() const;
#endif
	
private:
	FString CurrentLanguage = TEXT("en");
	
};
