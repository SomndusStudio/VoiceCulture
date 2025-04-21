/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "SSVoiceCultureSound.h"
#include "SSVoiceCultureEditorTypes.h"

class SSVOICECULTUREEDITOR_API FSSVoiceCultureUtils
{
public:

	/** Save the given package to disk (compatible with UE4 and UE5) */
	static bool SaveAsset(UPackage* Package, const FString& PackageFilename);
	
	/** Fills the LocalizedVoiceEntries array based on SoundBase assets following the naming convention: LVA_{lang}_{Suffix} */
	static bool AutoPopulateFromNaming(USSVoiceCultureSound* TargetAsset, const bool bShowSlowTask = true, const bool bShowNotify = true);
	
	static bool AutoPopulateFromVoiceActor(const FString& VoiceActorName, bool bOnlyMissingCulture = true);
	
	static void GenerateCultureCoverageReport(FSSVoiceCultureReport& OutReport);

	static bool LoadSavedCultureReport(FSSVoiceCultureReport& OutReport);
	
	static int32 AutoPopulateCulture(const FString& TargetCulture, bool bOverrideExisting);
	
	static void GenerateActorListJson();
};
