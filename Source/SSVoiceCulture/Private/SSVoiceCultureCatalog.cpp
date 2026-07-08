/**
* Copyright (C) Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#include "SSVoiceCultureCatalog.h"

#include "SSVoiceCultureSound.h"

void USSVoiceCultureCatalog::GatherSoftPathsForCulture(const FString& CultureCode, TArray<FSoftObjectPath>& OutPaths) const
{
	OutPaths.Reserve(OutPaths.Num() + VoiceCultureSounds.Num());

	for (const TObjectPtr<USSVoiceCultureSound>& VCSound : VoiceCultureSounds)
	{
		if (!VCSound) continue;

		for (const FSSCultureAudioEntry& Entry : VCSound->VoiceCultures)
		{
			if (Entry.Culture.Equals(CultureCode, ESearchCase::IgnoreCase) && !Entry.Sound.IsNull())
			{
				OutPaths.Add(Entry.Sound.ToSoftObjectPath());
				break; // one culture per VCSound
			}
		}
	}
}
