/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SSVoiceCultureSettings.generated.h"

// Notifies when PreviewLanguage changes
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPreviewLanguageChanged, const FString& /*NewLanguage*/);


/**
 * Settings class for managing voice culture configuration.
 * This appears under "Project Settings" using Unreal's UDeveloperSettings system.
 */
UCLASS(config=Game, defaultconfig)
class SSVOICECULTURE_API USSVoiceCultureSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	/**
	 * Constructor used to initialize default values or apply custom logic.
	 */
	USSVoiceCultureSettings(const FObjectInitializer& Initializer);

#if WITH_EDITOR
	virtual FText GetSectionText() const override;
#endif

public:

	static USSVoiceCultureSettings* GetMutableSetting();
	
	/**
	 * Static helper to retrieve the current settings instance.
	 * Can be called from anywhere to access project voice culture settings.
	 */
	static const USSVoiceCultureSettings* GetSetting();

	/**
	 * Default voice language used when no specific language is selected.
	 * Also acts as a fallback if a culture voice is missing.
	 */
	UPROPERTY(BlueprintReadOnly, Category="Voice Culture|General")
	FString DefaultLanguageFallback = TEXT("en");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Voice Culture|General")
	FString CurrentLanguage = TEXT("en");

	FString GetCurrentLanguage() const;
	
	/**
	 * If enabled, the preview language will be used during gameplay and PIE (Play In Editor).
	 * This is useful for testing culture voices without affecting the actual game language.
	 * 
	 * Only affects Editor / PIE environments.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Voice Culture|General")
	bool bUsePreviewLanguageInGame = false;
	
	/**
	 * The voice language used for preview when bUsePreviewLanguageInGame is enabled.
	 * Allows testing of different voice cultures during development.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Voice Culture|General")
	FString PreviewLanguage = TEXT("en");

	/**
	 * The list of supported voice cultures (languages) for the project.
	 * This is used to filter available voices and manage which ones are cooked or shipped.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Voice Culture|Cultures")
	TSet<FString> SupportedVoiceCultures;

	static FOnPreviewLanguageChanged OnPreviewLanguageChanged;
	
	// Call this whenever the PreviewLanguage changes
	static void SetPreviewLanguage(const FString& NewLanguage);

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
