/**
* Copyright (C) Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "SSVoiceCultureStatics.h"

#include "SSVoiceCultureSound.h"

float USSVoiceCultureStatics::GetVoiceCultureAwareDuration(const USoundBase* Sound)
{
	if (!Sound)
	{
		return 0.f;
	}

	// If it's a Voice Culture Sound, resolve the current culture's inner sound and query its duration.
	// This guarantees the returned duration matches the sound that will actually play right now,
	// even after a runtime culture change (since USoundBase::Duration is only refreshed on load/serialize).
	if (const USSVoiceCultureSound* VoiceCultureSound = Cast<USSVoiceCultureSound>(Sound))
	{
		if (const USoundBase* Inner = VoiceCultureSound->GetCurrentCultureSound())
		{
			return Inner->GetDuration();
		}
		return 0.f;
	}

	// Fallback: standard duration for any other sound type (SoundWave, SoundCue, MetaSound, ...)
	return Sound->GetDuration();
}

float USSVoiceCultureStatics::ResolveVoiceCultureAwareDuration(const USoundBase* Sound)
{
	return GetVoiceCultureAwareDuration(Sound);
}
