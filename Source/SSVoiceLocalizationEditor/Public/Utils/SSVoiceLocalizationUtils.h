// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SSLocalizedVoiceSound.h"

class SSVOICELOCALIZATIONEDITOR_API FSSVoiceLocalizationUtils
{
public:
	/** Fills the LocalizedVoiceEntries array based on SoundBase assets following the naming convention: LVA_{lang}_{Suffix} */
	static bool AutoPopulateFromNaming(USSLocalizedVoiceSound* TargetAsset);
};
