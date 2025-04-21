/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "SSVoiceCultureSound.h"
#include "UObject/Object.h"
#include "SSVoiceCultureStrategy.generated.h"

class USSVoiceCultureSound;

/**
 * Abstract base class for voice strategies.
 *
 * This class defines the interface for how voice culture audio assets are discovered and matched
 * to entries in a USSVoiceCultureSound asset. Strategies may vary based on naming conventions,
 * folder structures, metadata, etc.
 *
 * Concrete implementations can be created in Blueprint or C++ by subclassing this class.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class SSVOICECULTUREEDITOR_API USSVoiceCultureStrategy : public UObject
{
	GENERATED_BODY()

protected:
	/**
	 * Builds the expected asset suffix for a given culture and suffix.
	 * Each strategy can override this to define its naming convention.
	 * 
	 * Example:
	 * - Default: culture + "_" + suffix → "fr_NPC01_Hello"
	 * - CultureAtEnd: suffix + "_" + culture → "NPC01_Hello_fr"
	 */
	virtual FString BuildExpectedAssetSuffix(const FString& CultureCode, const FString& BaseSuffix) const;

	/**
	 * Parses the asset name into prefix, culture code, and suffix.
	 * The logic is strategy-dependent and can be overridden by child classes.
	 *
	 * @param AssetName     The full asset name (e.g., "A_EN_NPC01_Hello").
	 * @param OutPrefix     The extracted prefix (e.g., "A").
	 * @param OutCulture    The extracted culture code (e.g., "EN").
	 * @param OutSuffix     The suffix (e.g., "NPC01_Hello").
	 * @return true if the parsing succeeded and the format is valid.
	 */
	virtual bool ParseAssetName(const FString& AssetName, FString& OutPrefix, FString& OutCulture,
	                            FString& OutSuffix) const;

public:
	/**
	 * Returns a short description of the pattern used by this strategy to match voice cultures.
	 * This is used for UI feedback in dashboards or tooltips.
	 */
	UFUNCTION(BlueprintNativeEvent)
	FText DisplayMatchVoiceCulturePattern() const;

	/**
	 * Returns an example of how the voice culture match pattern looks for this strategy.
	 * Used to help users understand the expected format (e.g., "MyLine_en", "VO_001_fr").
	 */
	UFUNCTION(BlueprintNativeEvent)
	FText DisplayMatcVoiceCulturePatternExample() const;

	/**
	 * Returns a description of how this strategy recognizes culture codes from file/folder structure or asset naming.
	 */
	UFUNCTION(BlueprintNativeEvent)
	FText DisplayMatchCultureRulePattern() const;

	/**
	 * Returns an example of a culture matching rule in this strategy (e.g., file suffix "_fr", folder name "/fr").
	 */
	UFUNCTION(BlueprintNativeEvent)
	FText DisplayMatchCultureRulePatternExample() const;

	/**
	 * Finds a voice culture asset matching the given culture and suffix.
	 * Uses the current strategy's naming convention to locate the asset.
	 *
	 * @param CultureCode   The culture code to match (e.g. "en", "fr").
	 * @param Suffix        The base suffix to match against.
	 * @return The matching USoundBase asset, or nullptr if not found.
	 */
	virtual USoundBase* FindMatchingSoundAsset(const FString& CultureCode, const FString& Suffix) const;

	/**
	 * Executes the logic on a given base name, typically the name of a USSVoiceCultureSound asset.
	 * This function tries to locate all possible voice culture files that match the strategy rules.
	 *
	 * @param InBaseName The base name of the asset to match against (e.g. "MyLine").
	 * @param OutCultureToSound Output map of culture codes (e.g. "fr", "en") to matched USoundBase assets.
	 * @return true if at least one voice culture asset was matched.
	 */
	UFUNCTION(BlueprintNativeEvent)
	bool ExecuteAutoPopulate(const FString& InBaseName, TMap<FString, USoundBase*>& OutCultureToSound) const;

	virtual bool ExecuteOptimizedOneCultureAutoPopulateInAsset(
		USSVoiceCultureSound* TargetAsset,
		const FString& CultureCode,
		bool bOverrideExisting,
		FSSCultureAudioEntry& OutNewEntry,
		const TArray<FAssetData>& AssetCache) const;

	/**
	 * Executes a targeted AutoPopulate operation on a specific culture and asset.
	 * This allows fine-grained filling of a single language for a single asset.
	 *
	 * @param TargetAsset The USSVoiceCultureSound asset to AutoPopulate.
	 * @param CultureCode The culture code to match (e.g. "fr", "en").
	 * @param bOverrideExisting Whether to override an existing entry if one already exists.
	 * @param OutNewEntry Output entry that was added or modified.
	 * @return true if the entry was created or updated.
	 */
	UFUNCTION(BlueprintNativeEvent)
	bool ExecuteOneCultureAutoPopulateInAsset(
		USSVoiceCultureSound* TargetAsset,
		const FString& CultureCode,
		bool bOverrideExisting, FSSCultureAudioEntry& OutNewEntry
	);

	UFUNCTION(BlueprintNativeEvent)
	bool ExecuteExtractActorNameFromAsset(const FAssetData& AssetData, FString& OutActorName);

	UFUNCTION(BlueprintNativeEvent)
	bool ExecuteExtractActorNameFromAssetRegistry(TSet<FString>& OutUniqueActors);
};
