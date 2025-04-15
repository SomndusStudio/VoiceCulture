/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "SSVoiceLocalizationSubsystem.generated.h"

/**
 * Global engine subsystem that manages voice localization settings.
 *
 * Responsible for storing and resolving the current voice language used to pick localized voice audios.
 * 
 * Automatically resets to the default language (from developer settings) at the start of each game session,
 * without requiring a custom GameInstance.
 *
 * Can be queried or modified at runtime to switch voice language independently from UI/text localization.
 */
UCLASS()
class SSVOICELOCALIZATION_API USSVoiceLocalizationSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
	
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UFUNCTION(BlueprintCallable)
	void SetCurrentVoiceCulture(const FString& Language);

	UFUNCTION(BlueprintCallable)
	FString GetCurrentVoiceCulture() const;

#if WITH_EDITOR
	FString GetEditorPreviewLanguage() const;
#endif
	
private:
	FString CurrentLanguage = TEXT("en");

	void HandleStartGameInstance(UGameInstance* GameInstance);
	FDelegateHandle OnStartGameInstanceHandle;
};
