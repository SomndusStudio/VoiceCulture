/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "Settings/SSVoiceCultureStrategy_DefaultB.h"

FString USSVoiceCultureStrategy_DefaultB::BuildExpectedAssetSuffix(const FString& CultureCode,
	const FString& BaseSuffix) const
{
	return BaseSuffix + TEXT("_") + CultureCode.ToLower();
}

bool USSVoiceCultureStrategy_DefaultB::ParseAssetName(const FString& AssetName, FString& OutPrefix, FString& OutCulture,
                                                      FString& OutSuffix) const
{
	TArray<FString> Parts;
	AssetName.ParseIntoArray(Parts, TEXT("_"));

	if (Parts.Num() < 3)
		return false;

	OutPrefix = Parts[0];
	OutCulture = Parts.Last(); // Culture code is the last part

	// Rebuild suffix (between prefix and culture)
	OutSuffix.Empty();
	for (int32 i = 1; i < Parts.Num() - 1; ++i)
	{
		if (!OutSuffix.IsEmpty())
			OutSuffix += TEXT("_");

		OutSuffix += Parts[i];
	}

	return true;
}

FText USSVoiceCultureStrategy_DefaultB::DisplayMatchCultureRulePattern_Implementation() const
{
	return FText::FromString("{AssetType}_{ActorName}_{Suffix}_{Culture}");
}

FText USSVoiceCultureStrategy_DefaultB::DisplayMatchCultureRulePatternExample_Implementation() const
{
	return FText::FromString("A_NPC001_MarketScene01_L01_EN");
}
