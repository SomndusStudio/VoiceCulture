// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SSVoiceAutofillStrategy.generated.h"

/**
 * Abstract base class for voice autofill strategies.
 * Defines how voice assets are located and matched to a given localized voice sound.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class SSVOICELOCALIZATIONEDITOR_API USSVoiceAutofillStrategy : public UObject
{
	GENERATED_BODY()

public:
	/** 
	 * Called to search and match localized assets for a given base name.
	 * @param InBaseName The name of the LocalizedVoiceSound asset.
	 * @param OutCultureToSound Map of matched culture codes and their associated sound.
	 * @return Whether any result was found
	 */
	UFUNCTION(BlueprintNativeEvent)
	bool ExecuteAutofill(const FString& InBaseName, TMap<FString, USoundBase*>& OutCultureToSound) const;
};
