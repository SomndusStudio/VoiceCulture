/**
* Copyright (C) Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "SSVoiceCultureStrategy_Default.h"
#include "SSVoiceCultureStrategy_DefaultC.generated.h"

/**
 * 
 */
UCLASS()
class SSVOICECULTUREEDITOR_API USSVoiceCultureStrategy_DefaultC : public USSVoiceCultureStrategy_Default
{
	GENERATED_BODY()

public:
	
	USSVoiceCultureStrategy_DefaultC();
	
protected:

	virtual FString BuildExpectedAssetSuffix(const FString& CultureCode, const FString& BaseSuffix) const override;
	virtual bool ParseAssetName(const FString& AssetName, FString& OutPrefix, FString& OutCulture, FString& OutSuffix) const override;	
	
public:

	virtual FText DisplayMatchVoiceCulturePattern_Implementation() const override;
	virtual FText DisplayMatchVoiceCulturePatternExample_Implementation() const override;
	
	virtual FText DisplayMatchCultureRulePattern_Implementation() const override;
	virtual FText DisplayMatchCultureRulePatternExample_Implementation() const override;
};
