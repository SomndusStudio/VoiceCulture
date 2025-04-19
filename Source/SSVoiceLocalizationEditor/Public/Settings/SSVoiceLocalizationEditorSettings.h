/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "SSVoiceLocalizationTypes.h"
#include "Engine/DeveloperSettings.h"
#include "SSVoiceLocalizationEditorSettings.generated.h"

/**
 * Editor settings for the Voice Localization system (per-user project settings).
 *
 * These settings control the voice autofill profiles, active profile selection, and behavior of the Voice Dashboard tools.
 * Stored in EditorPerProjectUserSettings to allow each user to have independent configuration in the same project.
 */
UCLASS(config = EditorPerProjectUserSettings, defaultconfig, meta = (DisplayName = "Voice Autofill Settings"))
class SSVOICELOCALIZATIONEDITOR_API USSVoiceLocalizationEditorSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	
	/** Constructor used to apply default configuration. */
	USSVoiceLocalizationEditorSettings();
	
	/** Returns a const reference to the currently loaded editor settings object. */
	static const USSVoiceLocalizationEditorSettings* GetSetting();

	/** Returns a mutable version of the settings object (for runtime updates or Editor tools). */
	static USSVoiceLocalizationEditorSettings* GetMutableSetting();

	/**
	 * Fallback profile used when the active profile is not found or is invalid.
	 * Prevents crashes and ensures a minimum configuration exists.
	 */
	FSSVoiceAutofillProfile FallbackProfile;
	
	/**
	 * List of all available autofill profiles, shown in the Voice Dashboard.
	 * Each profile can define its own strategy, filters, or actor naming conventions.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Voice Autofill")
	TArray<FSSVoiceAutofillProfile> AutofillProfiles;

	/**
	 * Name of the currently active autofill profile.
	 * This determines which strategy is used for voice asset population.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Voice Autofill")
	FString ActiveVoiceProfileName;

	/**
	 * If enabled, voice assets will automatically be saved after an autofill operation completes.
	 * Useful for batch processing workflows or reducing manual saving effort.
	 */
	UPROPERTY(EditAnywhere, Config, Category="AutoFill")
	bool bAutoSaveAfterAutofill = true;
};
