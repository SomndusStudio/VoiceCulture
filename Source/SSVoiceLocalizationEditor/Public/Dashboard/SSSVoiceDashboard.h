// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SSVoiceLocalizationTypes.h"


class SSVOICELOCALIZATIONEDITOR_API SSSVoiceDashboard : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSSVoiceDashboard) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

protected:
	/** Last loaded report */
	FSSVoiceCultureReport CultureReport;
	
private:
	TSharedRef<SWidget> BuildLeftPanel();
	TSharedRef<SWidget> BuildRightPanel();
	TSharedRef<SWidget> BuildAssetList();

	TSharedRef<SWidget> BuildCultureListWidget();
	TSharedRef<SWidget> BuildCoverageSection();
};
