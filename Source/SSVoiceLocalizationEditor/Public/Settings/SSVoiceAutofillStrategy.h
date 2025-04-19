/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "SSLocalizedVoiceSound.h"
#include "UObject/Object.h"
#include "SSVoiceAutofillStrategy.generated.h"

class USSLocalizedVoiceSound;

/**
 * Abstract base class for voice autofill strategies.
 *
 * This class defines the interface for how localized voice audio assets are discovered and matched
 * to entries in a USSLocalizedVoiceSound asset. Strategies may vary based on naming conventions,
 * folder structures, metadata, etc.
 *
 * Concrete implementations can be created in Blueprint or C++ by subclassing this class.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class SSVOICELOCALIZATIONEDITOR_API USSVoiceAutofillStrategy : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Returns a short description of the pattern used by this strategy to match localized voices.
	 * This is used for UI feedback in dashboards or tooltips.
	 */
	UFUNCTION(BlueprintNativeEvent)
	FText DisplayMatchLocalizedVoicePattern() const;

	/**
	 * Returns an example of how the localized voice match pattern looks for this strategy.
	 * Used to help users understand the expected format (e.g., "MyLine_en", "VO_001_fr").
	 */
	UFUNCTION(BlueprintNativeEvent)
	FText DisplayMatchLocalizedVoicePatternExample() const;

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
	 * Executes the autofill logic on a given base name, typically the name of a USSLocalizedVoiceSound asset.
	 * This function tries to locate all possible localized voice files that match the strategy rules.
	 *
	 * @param InBaseName The base name of the asset to match against (e.g. "MyLine").
	 * @param OutCultureToSound Output map of culture codes (e.g. "fr", "en") to matched USoundBase assets.
	 * @return true if at least one localized voice asset was matched.
	 */
	UFUNCTION(BlueprintNativeEvent)
	bool ExecuteAutofill(const FString& InBaseName, TMap<FString, USoundBase*>& OutCultureToSound) const;

	/**
	 * Executes a targeted autofill operation on a specific culture and asset.
	 * This allows fine-grained filling of a single language for a single asset.
	 *
	 * @param TargetAsset The USSLocalizedVoiceSound asset to autofill.
	 * @param CultureCode The culture code to match (e.g. "fr", "en").
	 * @param bOverrideExisting Whether to override an existing entry if one already exists.
	 * @param OutNewEntry Output entry that was added or modified.
	 * @return true if the entry was created or updated.
	 */
	UFUNCTION(BlueprintNativeEvent)
	bool ExecuteOneCultureAutofillInAsset(
		USSLocalizedVoiceSound* TargetAsset,
		const FString& CultureCode,
		bool bOverrideExisting, FSSLocalizedAudioEntry& OutNewEntry
	);
};
