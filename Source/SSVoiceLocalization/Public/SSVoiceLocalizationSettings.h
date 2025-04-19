/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SSVoiceLocalizationSettings.generated.h"

/**
 * Settings class for managing voice localization configuration.
 * This appears under "Project Settings" using Unreal's UDeveloperSettings system.
 */
UCLASS(config=Game, defaultconfig)
class SSVOICELOCALIZATION_API USSVoiceLocalizationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	/**
	 * Constructor used to initialize default values or apply custom logic.
	 */
	USSVoiceLocalizationSettings(const FObjectInitializer& Initializer);

#if WITH_EDITOR
	virtual FText GetSectionText() const override;
#endif

public:
	
	/**
	 * Static helper to retrieve the current settings instance.
	 * Can be called from anywhere to access project voice localization settings.
	 */
	static const USSVoiceLocalizationSettings* GetSetting();

	/**
	 * Default voice language used when no specific language is selected.
	 * Also acts as a fallback if a localized voice is missing.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Voice Localization|General")
	FString DefaultLanguage = TEXT("en");

	/**
	 * If enabled, the preview language will be used during gameplay and PIE (Play In Editor).
	 * This is useful for testing localized voices without affecting the actual game language.
	 * 
	 * Only affects Editor / PIE environments.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Voice Localization|General")
	bool bUsePreviewLanguageInGame = false;

	/**
	 * The voice language used for preview when bUsePreviewLanguageInGame is enabled.
	 * Allows testing of different voice cultures during development.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Voice Localization|General")
	FString PreviewLanguage = TEXT("en");

	/**
	 * The list of supported voice cultures (languages) for the project.
	 * This is used to filter available voices and manage which ones are cooked or shipped.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Voice Localization|Cultures")
	TSet<FString> SupportedVoiceCultures;
};
