// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ContentBrowserModule.h"
#include "SSVoiceLocalizationTypes.h"
#include "Settings/SSVoiceAutofillStrategy.h"
#include "Subsystems/EngineSubsystem.h"
#include "SSVoiceLocalizationEditorSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class SSVOICELOCALIZATIONEDITOR_API USSVoiceLocalizationEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

protected:
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
public:

	void OnVoiceProfileNameChange();
	
	USSVoiceAutofillStrategy* RefreshStrategy();
	
	// Returns the currently active strategy (resolved)
	UFUNCTION(BlueprintCallable, Category="Voice Localization")
	USSVoiceAutofillStrategy* GetActiveStrategy();

	// Returns the current profile (from settings)
	const FSSVoiceAutofillProfile* GetActiveProfile() const;

	// Returns whether the subsystem is properly initialized
	bool IsReady() const;

	/**
	 * @brief Returns all Voice Actor assets in project
	 * @param Assets TArray<FAssetData>
	 * @param bShowSlowTask bool
	 */
	UFUNCTION(BlueprintCallable, Category="Voice Localization")
	static void GetAssetsFromVoiceActor(TArray<FAssetData>& Assets, FString VoiceActorName, const bool bShowSlowTask = true);

	static void GetAssetsWithCulture(TArray<FAssetData>& Assets, const bool bCompleteCulture = true, const bool bShowSlowTask = true);
	
	UFUNCTION(BlueprintCallable, Category="Voice Localization")
	static void GetAssetsWithMissingCulture(TArray<FAssetData>& Assets, const bool bShowSlowTask = true);

	UFUNCTION(BlueprintCallable, Category="Voice Localization")
    static void GetAssetsWithCompleteCulture(TArray<FAssetData>& Assets, const bool bShowSlowTask = true);
	
	static FContentBrowserModule& GetVoiceContentBrowser();
	
private:
	UPROPERTY(Transient)
	USSVoiceAutofillStrategy* CachedStrategy;
};
