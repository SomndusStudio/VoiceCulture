/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Settings/SSVoiceCultureStrategy.h"
#include "SSVoiceCultureEditorTypes.generated.h"

class USSVoiceCultureStrategy;

USTRUCT(BlueprintType)
struct FSSVoiceStrategyProfile
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Voice Culture")
	FString ProfileName;
	
	UPROPERTY(EditAnywhere, Category = "Voice Culture")
	TSoftClassPtr<USSVoiceCultureStrategy> StrategyClass;
};

/**
 * One culture scan result
 */
USTRUCT()
struct FSSVoiceCultureReportEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FString Culture;

	UPROPERTY()
	int32 TotalAssets = 0;
	
	UPROPERTY()
	int32 AssetsWithCulture = 0;

	/** Returns 0.0 - 1.0 coverage */
	float GetCoveragePercent() const
	{
		return TotalAssets > 0 ? float(AssetsWithCulture) / float(TotalAssets) : 0.0f;
	}
};

/**
 * Global voice culture status across cultures
 */
USTRUCT()
struct FSSVoiceCultureReport
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FSSVoiceCultureReportEntry> Entries;

	/** Timestamp of generation */
	UPROPERTY()
	FDateTime GeneratedAt;
};