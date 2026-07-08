/**
* Copyright (C) Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SSVoiceCultureCatalog.generated.h"

class USSVoiceCultureSound;

/**
 * A catalog of voice culture sounds that should be preloaded together as a batch
 * for a given voice culture (e.g. a scene, a chapter, a companion's full VO set).
 *
 * The catalog holds hard references to the VoiceCultureSound "index" assets, which are
 * lightweight - the actual audio remains behind soft references and is only loaded when
 * the catalog is asked to preload for a specific culture.
 */
UCLASS(BlueprintType)
class SSVOICECULTURE_API USSVoiceCultureCatalog : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Hard references to Voice Culture Sound assets to preload as a group. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice Culture", meta = (
		DisplayName = "Voice Culture Sounds",
		ToolTip = "The set of voice lines that belong together. All their culture-specific audio for the requested culture will be loaded as a single batch."))
	TArray<TObjectPtr<USSVoiceCultureSound>> VoiceCultureSounds;

	/**
	 * Collects all soft references to the actual audio assets for a given culture,
	 * across every VoiceCultureSound in the catalog. Missing entries are skipped silently.
	 */
	void GatherSoftPathsForCulture(const FString& CultureCode, TArray<FSoftObjectPath>& OutPaths) const;
};
