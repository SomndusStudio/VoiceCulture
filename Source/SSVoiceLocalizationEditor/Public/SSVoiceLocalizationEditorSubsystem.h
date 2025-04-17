// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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


private:
	UPROPERTY(Transient)
	USSVoiceAutofillStrategy* CachedStrategy;
};
