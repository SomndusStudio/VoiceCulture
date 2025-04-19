/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "SSVoiceAutofillStrategy.h"
#include "SSVoiceAutofillStrategy_Default.generated.h"

/**
 * Default implementation of voice autofill strategy:
 * Expects names in the format {Prefix}_{Culture}_{BaseSuffix}
 */
UCLASS(Blueprintable, EditInlineNew)
class SSVOICELOCALIZATIONEDITOR_API USSVoiceAutofillStrategy_Default : public USSVoiceAutofillStrategy
{
	GENERATED_BODY()

public:
	/** Culture code is expected at this index (e.g., 1 for "LVA_en_MyLine") */
	UPROPERTY(EditAnywhere, Category = "Strategy")
	int32 CultureIndex = 1;

	/** Allow filtering based on prefixes (optional) */
	UPROPERTY(EditAnywhere, Category = "Strategy")
	TArray<FString> AllowedPrefixes;

	/** Whether prefix filtering is case-sensitive */
	UPROPERTY(EditAnywhere, Category = "Strategy")
	bool bCaseSensitivePrefixes = false;

	UPROPERTY(EditAnywhere, Category = "Strategy")
	bool bRecursivePaths = true;
	
	UPROPERTY(EditAnywhere, Category = "Strategy")
	bool bRecursiveClasses = true;

	virtual FText DisplayMatchLocalizedVoicePattern_Implementation() const override;
	virtual FText DisplayMatchLocalizedVoicePatternExample_Implementation() const override;
	
	virtual FText DisplayMatchCultureRulePattern_Implementation() const override;
	virtual FText DisplayMatchCultureRulePatternExample_Implementation() const override;
	
	virtual bool ExecuteAutofill_Implementation(const FString& InBaseName, TMap<FString, USoundBase*>& OutCultureToSound) const override;
	virtual bool ExecuteOneCultureAutofillInAsset_Implementation(USSLocalizedVoiceSound* TargetAsset, const FString& CultureCode, bool bOverrideExisting, FSSLocalizedAudioEntry& OutNewEntry) override;


	virtual bool ExecuteExtractActorNameFromAsset_Implementation(const FAssetData& AssetData, FString& OutActorName) override;
	virtual bool ExecuteExtractActorNameFromAssetRegistry_Implementation(TSet<FString>& OutUniqueActors) override;
};
