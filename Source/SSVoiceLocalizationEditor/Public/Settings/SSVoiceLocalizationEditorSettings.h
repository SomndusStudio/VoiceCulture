// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SSVoiceLocalizationTypes.h"
#include "Engine/DeveloperSettings.h"
#include "SSVoiceLocalizationEditorSettings.generated.h"

/**
 * 
 */
UCLASS(config = EditorPerProjectUserSettings, defaultconfig, meta = (DisplayName = "Voice Autofill Settings"))
class SSVOICELOCALIZATIONEDITOR_API USSVoiceLocalizationEditorSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	/** List of all available autofill profiles used in the Voice Dashboard */
	UPROPERTY(EditAnywhere, config, Category = "Voice Autofill")
	TArray<FSSVoiceAutofillProfile> AutofillProfiles;
};
