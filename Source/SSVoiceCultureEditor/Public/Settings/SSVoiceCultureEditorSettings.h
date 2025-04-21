/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "SSVoiceCultureEditorTypes.h"
#include "Engine/DeveloperSettings.h"
#include "SSVoiceCultureEditorSettings.generated.h"

/**
 * Editor settings for the Voice Culture system (per-user project settings).
 *
 * These settings control the voice profiles, active profile selection, and behavior of the Voice Dashboard tools.
 * Stored in EditorPerProjectUserSettings to allow each user to have independent configuration in the same project.
 */
UCLASS(config = EditorPerProjectUserSettings, defaultconfig, meta = (DisplayName = "Voice Editor Settings"))
class SSVOICECULTUREEDITOR_API USSVoiceCultureEditorSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	
	/** Constructor used to apply default configuration. */
	USSVoiceCultureEditorSettings();
	
	/** Returns a const reference to the currently loaded editor settings object. */
	static const USSVoiceCultureEditorSettings* GetSetting();

	/** Returns a mutable version of the settings object (for runtime updates or Editor tools). */
	static USSVoiceCultureEditorSettings* GetMutableSetting();

	/**
	 * Fallback profile used when the active profile is not found or is invalid.
	 * Prevents crashes and ensures a minimum configuration exists.
	 */
	FSSVoiceStrategyProfile FallbackProfile;
	
	/**
	 * List of all available profiles, shown in the Voice Dashboard.
	 * Each profile can define its own strategy, filters, or actor naming conventions.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Profile")
	TArray<FSSVoiceStrategyProfile> StrategyProfiles;

	/**
	 * Name of the currently active profile.
	 * This determines which strategy is used for voice asset population.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Profile")
	FString ActiveVoiceProfileName;

	/**
	 * If enabled, voice assets will automatically be saved after an operation completes.
	 * Useful for batch processing workflows or reducing manual saving effort.
	 */
	UPROPERTY(EditAnywhere, Config, Category="Voice Culture")
	bool bAutoSaveAfterAutoPopulate = false;

	UPROPERTY(EditAnywhere, Config, Category="Voice Culture")
	bool bAutoPopulateOverwriteExisting = false;
};
