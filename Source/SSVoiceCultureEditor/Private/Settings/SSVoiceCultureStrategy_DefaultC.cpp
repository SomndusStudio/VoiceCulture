/**
* Copyright (C) Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#include "Settings/SSVoiceCultureStrategy_DefaultC.h"

USSVoiceCultureStrategy_DefaultC::USSVoiceCultureStrategy_DefaultC()
{
	bRequireSuffixAtEnd = false;
}

FString USSVoiceCultureStrategy_DefaultC::BuildExpectedAssetSuffix(const FString& CultureCode,
                                                                   const FString& BaseSuffix) const
{
	return BaseSuffix + TEXT("_") + CultureCode.ToLower();
}

bool USSVoiceCultureStrategy_DefaultC::ParseAssetName(const FString& AssetName, FString& OutPrefix, FString& OutCulture,
													  FString& OutSuffix) const
{
	TArray<FString> Parts;
	AssetName.ParseIntoArray(Parts, TEXT("_"));

	// Minimum expected format: Prefix_Suffix..._Culture (e.g. VC_11_Chapter_000_en)
	if (Parts.Num() < 3)
		return false;

	// The culture code is the last part: it must be purely alphabetic (2 to 5 letters).
	// This avoids mistaking "000" (VO_11_Chapter_000) for a culture code.
	const FString& LastPart = Parts.Last();

	const bool bIsCultureCode = LastPart.Len() >= 2 && LastPart.Len() <= 5 &&
		!LastPart.IsEmpty() && [&LastPart]()
		{
			for (const TCHAR C : LastPart)
			{
				if (!FChar::IsAlpha(C))
					return false;
			}
			return true;
		}();

	if (!bIsCultureCode)
		return false; // e.g. "VO_11_Chapter_000" → not a culture asset, reject it

	OutPrefix = Parts[0];
	OutCulture = LastPart;

	// Rebuild the suffix (between the prefix and the culture) → "11_Chapter_000"
	OutSuffix.Empty();
	for (int32 i = 1; i < Parts.Num() - 1; ++i)
	{
		if (!OutSuffix.IsEmpty())
			OutSuffix += TEXT("_");

		OutSuffix += Parts[i];
	}

	return true;
}

FText USSVoiceCultureStrategy_DefaultC::DisplayMatchVoiceCulturePattern_Implementation() const
{
	return FText::FromString("VO_{ChapterCount}_Chapter_{Suffix}");
}

FText USSVoiceCultureStrategy_DefaultC::DisplayMatchVoiceCulturePatternExample_Implementation() const
{
	return FText::FromString("VO_11_Chapter_000");
}

FText USSVoiceCultureStrategy_DefaultC::DisplayMatchCultureRulePattern_Implementation() const
{
	return FText::FromString("{AssetType}_{ChapterCount}_Chapter_{Suffix}_{Culture}");
}

FText USSVoiceCultureStrategy_DefaultC::DisplayMatchCultureRulePatternExample_Implementation() const
{
	return FText::FromString("VC_11_Chapter_000_en");
}
