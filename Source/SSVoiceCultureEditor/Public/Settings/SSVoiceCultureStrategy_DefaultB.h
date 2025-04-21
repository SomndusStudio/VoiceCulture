/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "SSVoiceCultureStrategy_Default.h"
#include "SSVoiceCultureStrategy_DefaultB.generated.h"

/**
 * 
 */
UCLASS()
class SSVOICECULTUREEDITOR_API USSVoiceCultureStrategy_DefaultB : public USSVoiceCultureStrategy_Default
{
	GENERATED_BODY()

protected:

	virtual FString BuildExpectedAssetSuffix(const FString& CultureCode, const FString& BaseSuffix) const override;
	virtual bool ParseAssetName(const FString& AssetName, FString& OutPrefix, FString& OutCulture, FString& OutSuffix) const override;
	
public:
	virtual FText DisplayMatchCultureRulePattern_Implementation() const override;
	virtual FText DisplayMatchCultureRulePatternExample_Implementation() const override;
};
