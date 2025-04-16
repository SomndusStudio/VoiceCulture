// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Settings/SSVoiceLocalizationEditorSettings.h"

////////////////////////////////////////////////////////////////////
// Autofill Profile Selector

/**
 * Reusable dropdown widget to select an Autofill Profile
 */
class SSVOICELOCALIZATIONEDITOR_API SSSAutofillProfileSelector : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSSAutofillProfileSelector) {}
	SLATE_EVENT(FSimpleDelegate, OnProfileChanged)
SLATE_END_ARGS()

void Construct(const FArguments& InArgs);

	/** Returns currently selected profile (can be null) */
	TSharedPtr<FSSVoiceAutofillProfile> GetSelectedProfile() const;

private:
	void LoadProfiles();

	TSharedRef<SWidget> GenerateProfileItem(TSharedPtr<FSSVoiceAutofillProfile> InItem);
	void HandleProfileChanged(TSharedPtr<FSSVoiceAutofillProfile> NewSelection, ESelectInfo::Type);

	TArray<TSharedPtr<FSSVoiceAutofillProfile>> ProfileOptions;
	TSharedPtr<FSSVoiceAutofillProfile> CurrentSelection;
	TSharedPtr<SComboBox<TSharedPtr<FSSVoiceAutofillProfile>>> ComboBox;

	FSimpleDelegate OnProfileChangedDelegate;
};