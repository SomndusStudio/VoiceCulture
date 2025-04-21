/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "SSVoiceCultureSubsystem.generated.h"

/**
 * Global engine subsystem that manages voice culture settings.
 *
 * Responsible for storing and resolving the current voice language used to pick culture voice audios.
 * 
 * Automatically resets to the default language (from developer settings) at the start of each game session,
 * without requiring a custom GameInstance.
 *
 * Can be queried or modified at runtime to switch voice language independently from UI/text localization.
 */
UCLASS()
class SSVOICECULTURE_API USSVoiceCultureSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
	
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Sets the current voice culture (language) to use for culture voice audio.
	 * This can be called at runtime to switch voices independently from UI localization.
	 *
	 * @param Language The culture/language code to apply (e.g., "en", "fr", "jp").
	 */
	UFUNCTION(BlueprintCallable)
	void SetCurrentVoiceCulture(const FString& Language);

	/**
	 * Returns the currently active voice culture.
	 *
	 * @return The language code currently used for selecting culture voice assets.
	 */
	UFUNCTION(BlueprintCallable)
	FString GetCurrentVoiceCulture() const;

#if WITH_EDITOR
	/**
	 * In editor only: returns the preview voice culture defined in the developer settings.
	 * This is used for voice testing in PIE or the editor environment.
	 */
	FString GetEditorPreviewLanguage() const;
#endif
	
private:
	/** Holds the currently active voice language code. */
	FString CurrentLanguage = TEXT("en");

	/**
	 * Internal handler that is called at the start of each GameInstance.
	 * Resets the voice culture to the default (from developer settings) or preview language if in PIE.
	 */
	void HandleStartGameInstance(UGameInstance* GameInstance);

	/** Handle for the delegate binding to GameInstance start events. */
	FDelegateHandle OnStartGameInstanceHandle;
};
