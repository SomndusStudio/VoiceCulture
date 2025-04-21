/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "SSVoiceCultureStrategy.h"
#include "SSVoiceCultureStrategy_Default.generated.h"

/**
 * Default implementation of voice AutoPopulate strategy:
 * Expects names in the format {Prefix}_{Culture}_{BaseSuffix}
 */
UCLASS(Blueprintable, EditInlineNew)
class SSVOICECULTUREEDITOR_API USSVoiceCultureStrategy_Default : public USSVoiceCultureStrategy
{
	GENERATED_BODY()

protected:

	virtual FString BuildExpectedAssetSuffix(const FString& CultureCode, const FString& BaseSuffix) const override;
	
	/**
	 * Extracts the suffix portion of a base asset name by removing the prefix and culture code.
	 * Example: "A_EN_NPC01_Scene01" → "NPC01_Scene01"
	 */
	virtual FString ExtractSuffixFromBaseName(const FString& BaseName) const;

	/**
	 * Parses an asset name into its prefix, culture code, and suffix components.
	 * Example: "A_EN_NPC01_Scene01" → "A", "EN", "NPC01_Scene01"
	 *
	 * @param AssetName     The full name of the asset (e.g. "A_EN_NPC01_Scene01").
	 * @param OutPrefix     The extracted prefix (e.g. "A").
	 * @param OutCulture    The extracted culture code (e.g. "EN").
	 * @param OutSuffix     The suffix after the culture code (e.g. "NPC01_Scene01").
	 * @return true if the name was successfully parsed; false otherwise.
	 */
	virtual bool ParseAssetName(const FString& AssetName, FString& OutPrefix, FString& OutCulture, FString& OutSuffix) const override;
	
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

	virtual FText DisplayMatchVoiceCulturePattern_Implementation() const override;
	virtual FText DisplayMatchVoiceCulturePatternExample_Implementation() const override;

	virtual FText DisplayMatchCultureRulePattern_Implementation() const override;
	virtual FText DisplayMatchCultureRulePatternExample_Implementation() const override;

	virtual bool ExecuteAutoPopulate_Implementation(const FString& InBaseName,
	                                            TMap<FString, USoundBase*>& OutCultureToSound) const override;

	virtual bool ExecuteOptimizedOneCultureAutoPopulateInAsset(USSVoiceCultureSound* TargetAsset, const FString& CultureCode, bool bOverrideExisting, FSSCultureAudioEntry& OutNewEntry, const TArray<FAssetData>& AssetCache) const override;


	virtual bool
	ExecuteExtractActorNameFromAsset_Implementation(const FAssetData& AssetData, FString& OutActorName) override;
	virtual bool ExecuteExtractActorNameFromAssetRegistry_Implementation(TSet<FString>& OutUniqueActors) override;
};
