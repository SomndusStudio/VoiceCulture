/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "SSLocalizedVoiceSound.h"
#include "SSVoiceLocalizationTypes.h"

class SSVOICELOCALIZATIONEDITOR_API FSSVoiceLocalizationUtils
{
public:
	
	/** Fills the LocalizedVoiceEntries array based on SoundBase assets following the naming convention: LVA_{lang}_{Suffix} */
	static bool AutoPopulateFromNaming(USSLocalizedVoiceSound* TargetAsset, const bool bShowSlowTask = true, const bool bShowNotify = true);
	
	static bool AutoPopulateFromVoiceActor(const FString& VoiceActorName, bool bOnlyMissingCulture = true);
	
	static void GenerateCultureCoverageReportAsync(TFunction<void(const FSSVoiceCultureReport&)> OnComplete);

	static bool LoadSavedCultureReport(FSSVoiceCultureReport& OutReport);
	
	static void AutoFillCultureAsync(const FString& TargetCulture, bool bOverrideExisting,
	                          TFunction<void(int32 ModifiedAssetCount)> OnCompleted);
	static void GenerateActorListJson();
};
